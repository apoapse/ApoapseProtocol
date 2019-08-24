#include "stdafx.h"
#include "Common.h"
#include "FileStreamConnection.h"
#include "CryptographyTypes.hpp"
#include "Username.h"
#include <filesystem>

FileStreamConnection::FileStreamConnection(io_context& ioService, ssl::context& context) : TCPConnection(ioService, context)
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

void FileStreamConnection::HandleFileWriteAsync(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnection> tcpConnection)
{
	ASSERT(IsSendingFile());
	//LOG_DEBUG << "HandleFileWriteAsync" << bytesTransferred;

	m_currentFileSend->sentSize += (UInt32)bytesTransferred;

	if (m_currentFileSend->sentSize == m_currentFileSend->fileSize)
	{
		m_currentFileSend->readStream.close();
		m_currentFileSend.reset();

		OnFileSentInternal();
		StartReading();
	}
	else
	{
		m_currentFileSend->readStream.read((char*)m_writeBuffer.data(), m_writeBuffer.size());
		std::streamsize dataSize = m_currentFileSend->readStream.gcount();

		auto self(shared_from_this());
		m_socket->async_write_some(boost::asio::buffer(m_writeBuffer, dataSize), [this, self](boost::system::error_code er, size_t bytesTransferred)
		{
			HandleFileWriteAsync(er, bytesTransferred, self);
		});
	}
}

void FileStreamConnection::OnSendingSuccessful(size_t bytesTransferred)
{
	StartReading();
}

void FileStreamConnection::StartReading()
{
	ReadSome(m_readBuffer, [this](size_t bytesTransferred) { OnReceiveData(bytesTransferred); });
}

void FileStreamConnection::OnReceiveData(size_t bytesTransferred)
{
	if (global->isServer && !m_socketAuthenticated)
	{
		if (bytesTransferred == (sha256Length + sha256Length))	// header + Username + auth code
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
		LOG << "Start file download";
		m_currentFileDownload = FileReceive();
	}

	Range data(m_readBuffer, bytesTransferred);

	m_currentFileDownload->receivedSize += static_cast<UInt32>(data.size());

	if (!m_currentFileDownload->headerReceived)
	{
		if (m_currentFileDownload->receivedSize <= FileReceive::headerLength)
		{
			const UInt64 receivedHeaderLength = std::min(data.size(), (size_t)FileReceive::headerLength);
			std::copy(data.begin(), data.begin() + receivedHeaderLength, m_currentFileDownload->headerData.begin());
			data.Consume(receivedHeaderLength);
		}
		else if (m_currentFileDownload->receivedSize >= FileReceive::headerLength)
		{
			OnFullHeaderReceived();
		}
	}

	if (data.size() > 0)
	{
		OnFilePartReceived(data);
	}

	StartReading();
}

void FileStreamConnection::OnFullHeaderReceived()
{
	ASSERT(IsDownloadingFile());
	ASSERT(m_currentFileDownload->receivedSize >= FileReceive::headerLength);

	const auto& currentFile = m_filesToReceiveQueue.front();
	m_currentFileDownload->fileSize = FromBytes<UInt32>(m_currentFileDownload->headerData, Endianness::BIG_ENDIAN);
	m_currentFileDownload->receivedSize = m_currentFileDownload->receivedSize - FileReceive::headerLength;

	if (m_currentFileDownload->fileSize != currentFile.fileSize)
		throw std::exception("The file size do not match");

	// We create the parent directory if it does not exist
	const std::string filePathFolder = std::filesystem::path(currentFile.filePath).parent_path().string();
	if (!filePathFolder.empty() && !std::filesystem::exists(filePathFolder))
	{
		std::filesystem::create_directory(filePathFolder);
	}

	m_currentFileDownload->writeStream = std::ofstream(currentFile.filePath, std::ofstream::binary);

	m_currentFileDownload->headerReceived = true;
	LOG << "Start downloading file of size " << m_currentFileDownload->fileSize << " to " << currentFile.filePath;
}

void FileStreamConnection::OnFilePartReceived(Range<std::array<byte, FILE_STREAM_READ_BUFFER_SIZE>> data)
{
	const UInt32 bytesToWrite = std::min((UInt32)data.size(), (m_currentFileDownload->fileSize - m_currentFileDownload->writtenSize));

	m_currentFileDownload->writeStream.write((const char*)data.data(), bytesToWrite);
	
	m_currentFileDownload->writtenSize += bytesToWrite;
	//LOG_DEBUG << "OnFilePartReceived. Size written" << m_currentFileDownload->writtenSize << " of " << m_currentFileDownload->fileSize;

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
	LOG << error.message() << LogSeverity::warning;

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
	m_currentFileSend->fileSize = static_cast<UInt32>(size + (UInt64)FileReceive::headerLength);
	
	m_currentFileSend->readStream = std::ifstream(filePath, std::ifstream::binary);

	auto headerData = ToBytes<UInt32>((UInt32)size, Endianness::BIG_ENDIAN);
	ASSERT(headerData.size() == FileReceive::headerLength);
	std::copy(headerData.begin(), headerData.end(), m_writeBuffer.begin());

	auto self(shared_from_this());
	m_socket->async_write_some(boost::asio::buffer(m_writeBuffer, headerData.size()), [this, self](boost::system::error_code er, size_t bytesTransferred)
	{
		HandleFileWriteAsync(er, bytesTransferred, self);
	});
}

void FileStreamConnection::OnFileSentInternal()
{
	const AttachmentFile fileSent = m_filesToSendQueue.front();
	m_filesToSendQueue.pop_front();
	
	OnFileSentSuccessfully(fileSent);

	if (!m_filesToSendQueue.empty())
		SendFileFromQueue();
}
