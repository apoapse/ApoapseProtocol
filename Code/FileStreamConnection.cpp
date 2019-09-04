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

	auto& file = m_filesToSendQueue.front();
	SendFileInternal(file.filePath, file.fileSize);
}

void FileStreamConnection::SendFileInternal(const std::string& filePath, UInt64 size)
{
	LOG << "Sending file " << filePath << " size: " << size;
	
	m_currentFileSend = FileSend();
	m_currentFileSend->fileSize = static_cast<UInt32>(size);
	m_currentFileSend->readStream = std::ifstream(filePath, std::ifstream::binary);

	// Read file
	m_currentFileSend->readStream.read((char*)m_writeBuffer.data(), m_writeBuffer.size());
	const std::streamsize dataSize = m_currentFileSend->readStream.gcount();

	LOG_DEBUG << "Read " << dataSize << " bytes from file";

	auto self(shared_from_this());
	m_socket->async_write_some(boost::asio::buffer(m_writeBuffer, dataSize), boost::asio::bind_executor(m_strand, [this, self](boost::system::error_code er, size_t bytesTransferred)
	{
		HandleFileWriteAsync(er, bytesTransferred, self);
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

void FileStreamConnection::HandleFileWriteAsync(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnectionNoTLS> tcpConnection)
{
	ASSERT(IsSendingFile());
	m_currentFileSend->sentSize += (UInt32)bytesTransferred;

	//LOG_DEBUG << "HandleFileWriteAsync " << m_currentFileSend->sentSize << " of " << m_currentFileSend->fileSize << " thread: " << ThreadUtils::GetThreadName();
	
	if (m_currentFileSend->sentSize == m_currentFileSend->fileSize || bytesTransferred == 0)
	{
		m_currentFileSend->readStream.close();
		OnFileSentInternal();

		m_currentFileSend.reset();
		StartReading();
	}
	else
	{
		std::this_thread::sleep_for(2ms);
		m_currentFileSend->readStream.read((char*)m_writeBuffer.data(), m_writeBuffer.size());
		const std::streamsize dataSize = m_currentFileSend->readStream.gcount();

		//LOG_DEBUG << "Read " << dataSize << " bytes from file";
		auto self(shared_from_this());
		m_socket->async_write_some(boost::asio::buffer(m_writeBuffer, dataSize), boost::asio::bind_executor(m_strand, [this, self](boost::system::error_code er, size_t bytesTransferred)
		{
			HandleFileWriteAsync(er, bytesTransferred, self);
		}));
	}
}

void FileStreamConnection::OnSendingSuccessful(size_t bytesTransferred)
{
	StartReading();
}

void FileStreamConnection::StartReading()
{
	ReadSome(m_readBuffer, [this](size_t bytesTransferred)
	{
		OnReceiveData(bytesTransferred);
	});
}

void FileStreamConnection::OnReceiveData(size_t bytesTransferred)
{
	if (global->isServer && !m_socketAuthenticated)
	{
		if (bytesTransferred == (sha256Length + sha256Length))	// Username + auth code
		{
			Range range(m_readBuffer, bytesTransferred);
			
			const Username username = Username(ByteContainer(range.begin(), range.begin() + sha256Length));
			range.Consume(sha256Length);
			
			hash_SHA256 authCode;
			std::copy(range.begin(), range.begin() + sha256Length, authCode.begin());
			
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

	if (!IsDownloadingFile())
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

	Range data(m_readBuffer, bytesTransferred);

	m_currentFileDownload->receivedSize += static_cast<UInt32>(data.size());

	if (data.size() > 0)
	{
		OnFilePartReceived(data);
	}

	StartReading();
}

void FileStreamConnection::OnFilePartReceived(Range<std::array<byte, FILE_STREAM_READ_BUFFER_SIZE>> data)
{
	const UInt32 bytesToWrite = std::min((UInt32)data.size(), (m_currentFileDownload->fileSize - m_currentFileDownload->writtenSize));
	//LOG_DEBUG << std::string(data.begin(), data.begin() + bytesToWrite);

	std::array<byte, FILE_STREAM_READ_BUFFER_SIZE> localBuffer;
	std::copy(m_readBuffer.begin(), m_readBuffer.begin() + bytesToWrite, localBuffer.begin());
	
	m_currentFileDownload->writeStream.write((const char*)localBuffer.data(), bytesToWrite);
	m_currentFileDownload->writtenSize += bytesToWrite;
	
	if (m_currentFileDownload->writtenSize == m_currentFileDownload->fileSize)
	{
		m_currentFileDownload->writeStream.close();

		const AttachmentFile file = m_filesToReceiveQueue.front();
		OnFileDownloadCompleted(file);

		m_filesToReceiveQueue.pop_front();
		m_currentFileDownload.reset();
	}
}

bool FileStreamConnection::OnSocketConnectedInternal()
{
	StartReading();
	OnSocketConnected();
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