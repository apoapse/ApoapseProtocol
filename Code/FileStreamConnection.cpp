#include "stdafx.h"
#include "Common.h"
#include "FileStreamConnection.h"
#include "CryptographyTypes.hpp"
#include "Username.h"
#include <filesystem>
#include "ThreadUtils.h"

AttachmentFile::AttachmentFile(DataStructure& data, const std::string& filePath)
{
	uuid = data.GetField("uuid").GetValue<Uuid>();
	relatedMessage = data.GetField("parent_message").GetValue<Uuid>();
	fileName = data.GetField("name").GetValue<std::string>();
	fileSize = data.GetField("file_size").GetValue<Int64>();
	this->filePath = filePath;
}

FileStreamConnection::FileStreamConnection(io_context& ioService/*, ssl::context& context*/) : TCPConnectionNoTLS(ioService/*, context*/), m_strand(ioService)
{
}

FileStreamConnection::~FileStreamConnection()
{
	LOG_DEBUG << "~FileStreamConnection";
}

bool FileStreamConnection::IsDownloadingFile() const
{
	return m_currentFileDownload.has_value();
}

bool FileStreamConnection::IsSendingFile() const
{
	return m_currentFileSend.has_value();
}

void FileStreamConnection::SendFileFromQueue()
{
	ASSERT(!m_filesToSendQueue.empty());

	m_socket->get_io_service().post([this]()
	{
		auto& file = m_filesToSendQueue.front();
		SendFileInternal(file.filePath, file.fileSize);
	});
}

void FileStreamConnection::SendFileInternal(const std::string& filePath, UInt64 size)
{
	m_currentFileSend = FileSend();
	m_currentFileSend->fileSize = static_cast<UInt32>(size);
	m_currentFileSend->readStream = std::ifstream(filePath, std::ifstream::binary);

	LOG << "Sending file " << filePath << " size: " << m_currentFileSend->fileSize;

	std::shared_ptr<WriteBuffer> newBuffer = ReadFromFile();

	auto self(shared_from_this());
	m_socket->async_write_some(boost::asio::buffer(newBuffer->data), boost::asio::bind_executor(m_strand, [this, self, newBuffer](boost::system::error_code er, size_t bytesTransferred)
	{
		HandleFileWriteAsync(er, bytesTransferred, newBuffer, self);
	}));
}

void FileStreamConnection::OnFileSentInternal()
{
	const AttachmentFile fileSent = m_filesToSendQueue.front();
	m_filesToSendQueue.pop_front();
	
	OnFileSentSuccessfully(fileSent);

	if (!m_filesToSendQueue.empty())
		SendFileFromQueue();
}

std::shared_ptr<FileStreamConnection::WriteBuffer> FileStreamConnection::ReadFromFile()
{
	auto buffer = std::make_shared<WriteBuffer>();
	buffer->index = m_currentFileSend->packetIndex;
	m_currentFileSend->packetIndex++;

	m_currentFileSend->readStream.read((char*)&buffer->data, buffer->data.size());
	const std::streamsize dataSize = m_currentFileSend->readStream.gcount();
	return buffer;
}

void FileStreamConnection::HandleFileWriteAsync(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<WriteBuffer> buffer, std::shared_ptr<TCPConnectionNoTLS> tcpConnection)
{
	ASSERT(IsSendingFile());
	//LOG_DEBUG << "HandleFileWriteAsync " << m_currentFileSend->sentSize << " of " << m_currentFileSend->fileSize << " index: " << buffer->index;

	m_currentFileSend->sentSize += bytesTransferred;
	
	if (m_currentFileSend->sentSize >= m_currentFileSend->fileSize)
	{
		m_currentFileSend->readStream.close();
		OnFileSentInternal();
		m_currentFileSend.reset();

		StartReading();
	}
	else
	{
		std::this_thread::sleep_for(4ms);	// TODO IMPORTANT THIS THING IS NECESSARY FOR NOW TO HAVE DATA RECEIVED IN THE RIGHT ORDER
		std::shared_ptr<WriteBuffer> newBuffer = ReadFromFile();

		auto self(shared_from_this());
		m_socket->async_write_some(boost::asio::buffer(newBuffer->data), boost::asio::bind_executor(m_strand, [this, self, newBuffer](boost::system::error_code er, size_t bytesTransferred)
		{
			HandleFileWriteAsync(er, bytesTransferred, newBuffer, self);
		}));
	}
}

void FileStreamConnection::OnSendingSuccessful(size_t bytesTransferred)
{
	StartReading();
}

void FileStreamConnection::StartReading()
{
	auto self(shared_from_this());
	m_socket->async_read_some(boost::asio::buffer(m_readBuffer), boost::asio::bind_executor(m_strand, [this, self](boost::system::error_code er, size_t bytesTransferred)
	{
		OnReceiveData(bytesTransferred, self);
	}));
}

void FileStreamConnection::OnReceiveData(size_t bytesTransferred, std::shared_ptr<TCPConnectionNoTLS> tcpConnection)
{
	std::shared_ptr<NetBuffer> dataBuffer = std::make_shared<NetBuffer>();
	std::copy(m_readBuffer.begin(), m_readBuffer.begin() + bytesTransferred, dataBuffer->begin());
	Range data(*dataBuffer, bytesTransferred);
	
	if (global->isServer && !m_socketAuthenticated)
	{
		if (bytesTransferred == (sha256Length + sha256Length))	// Username + auth code
		{
			const Username username = Username(ByteContainer(data.begin(), data.begin() + sha256Length));
			data.Consume(sha256Length);
			
			hash_SHA256 authCode;
			std::copy(data.begin(), data.begin() + sha256Length, authCode.begin());
			
			Authenticate(username, authCode);
		}
		else
		{
			LOG << LogSeverity::error << "Authentication message expected but the data received is not of the right size (" << bytesTransferred << ")";
			ErrorDisconnectAll();
		}

		StartReading();
		return;
	}

	if (!IsDownloadingFile() && !m_filesToReceiveQueue.empty())
	{
		auto& file = m_filesToReceiveQueue.front();
		m_currentFileDownload = FileReceive();
		m_currentFileDownload->fileSize = (UInt32)file.fileSize;

		// We create the parent directory if it does not exist
		const std::string filePathFolder = std::filesystem::path(file.filePath).parent_path().string();
		if (!filePathFolder.empty() && !std::filesystem::exists(filePathFolder))
		{
			std::filesystem::create_directory(filePathFolder);
		}

		m_currentFileDownload->writeStream = std::ofstream(file.filePath, std::ofstream::binary);

		LOG << "Start file download (size: " << m_currentFileDownload->fileSize << ")";
	}

	if (IsDownloadingFile() && bytesTransferred > 0)
	{
		OnFilePartReceived(bytesTransferred, dataBuffer);
	}
}

void FileStreamConnection::OnFilePartReceived(size_t bytesTransferred, std::shared_ptr<NetBuffer> dataBuffer)
{
	Range data(*dataBuffer, bytesTransferred);
	const UInt32 bytesToWrite = std::min((UInt32)data.size(), (m_currentFileDownload->fileSize - m_currentFileDownload->writtenSize));

	m_currentFileDownload->writtenSize += bytesToWrite;

	auto& bufferRef = *dataBuffer;
	m_currentFileDownload->writeStream.write((const char*)&bufferRef, bytesToWrite);

	//LOG_DEBUG << "HandleFileWriteAsync " << m_currentFileDownload->writtenSize << " of " << m_currentFileDownload->fileSize/* << " index: " << packetIndex*/;
	
	if (m_currentFileDownload->writtenSize == m_currentFileDownload->fileSize)
	{
		m_currentFileDownload->writeStream.close();
		const AttachmentFile file = m_filesToReceiveQueue.front();

		OnFileDownloadCompleted(file);

		m_filesToReceiveQueue.pop_front();
		m_currentFileDownload.reset();

		StartReading();
	}
	else
	{
		auto self(shared_from_this());
		if (m_socket->available())
		{
			boost::asio::async_read(*m_socket, boost::asio::buffer(m_readBuffer), boost::asio::transfer_all(), boost::asio::bind_executor(m_strand, [this, self](boost::system::error_code er, size_t bytesTransferred)
			{
				OnReceiveData(bytesTransferred, self);
			}));
		}
		else
		{
			StartReading();
		}
	}
}

bool FileStreamConnection::OnSocketConnectedInternal()
{
	OnSocketConnected();
	StartReading();
	return true;
}

bool FileStreamConnection::OnReceivedError(const boost::system::error_code& error)
{
	LOG << "OnReceivedError " << error.message() << LogSeverity::warning;

	return true;
}

void FileStreamConnection::PushFileToReceive(const AttachmentFile& file)
{
	m_filesToReceiveQueue.push_back(file);
}

void FileStreamConnection::PushFileToSend(const AttachmentFile& file)
{
	m_filesToSendQueue.push_back(file);

	SendFileFromQueue();
}

void FileStreamConnection::SetAuthenticated()
{
	m_socketAuthenticated = true;
}