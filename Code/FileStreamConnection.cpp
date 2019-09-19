#include "stdafx.h"
#include "Common.h"
#include "FileStreamConnection.h"
#include "CryptographyTypes.hpp"
#include "Username.h"
#include <filesystem>

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

void FileStreamConnection::SetCustomTCPOptions()
{
	/*{
		const boost::asio::socket_base::send_buffer_size option(FILE_STREAM_READ_BUFFER_SIZE);
		GetSocket().set_option(option);
	}

	{
		const boost::asio::socket_base::receive_buffer_size option(FILE_STREAM_READ_BUFFER_SIZE);
		GetSocket().set_option(option);
	}

	GetSocket().set_option(boost::asio::ip::tcp::no_delay(true));*/
}

bool FileStreamConnection::OnSocketConnectedInternal()
{
/*	{
		boost::asio::socket_base::receive_buffer_size option;
		GetSocket().get_option(option);
		LOG_DEBUG << "receive_buffer_size "<< option.value();
	}

	{
		boost::asio::socket_base::send_buffer_size option;
		GetSocket().get_option(option);
		LOG_DEBUG << "send_buffer_size "<< option.value();
	}
	*/
	OnSocketConnected();

	// Start read auth code
	if (global->isServer)
	{
		auto self(shared_from_this());
		boost::asio::async_read(*m_socket, boost::asio::buffer(m_readBuffer), boost::asio::transfer_exactly(sha256Length + sha256Length), boost::asio::bind_executor(m_strand, [this, self](boost::system::error_code er, size_t bytesTransferred)
		{
			OnReceiveAuthCode(er, bytesTransferred, self);
		}));
	}
	else
	{
		ListenForNewData();
	}
	
	return true;
}

/////////////////////////////// DOWNLOAD //////////////////////////////////
void FileStreamConnection::ListenForNewData()
{
	auto self(shared_from_this());
	m_socket->async_read_some(boost::asio::buffer(m_readBuffer), boost::asio::bind_executor(m_strand, [this, self](boost::system::error_code er, size_t bytesTransferred)
	{
		OnReceiveData(er, bytesTransferred, self);
	}));
}

void FileStreamConnection::ListenExactly(Int64 size)
{
	auto self(shared_from_this());
	boost::asio::async_read(*m_socket, boost::asio::buffer(m_readBuffer), boost::asio::transfer_exactly(size), boost::asio::bind_executor(m_strand, [this, self](boost::system::error_code er, size_t bytesTransferred)
	{
		OnReceiveData(er, bytesTransferred, self);
	}));
}

void FileStreamConnection::OnReceiveAuthCode(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnectionNoTLS> TCPConnectionNoTLS)
{
	if (error)
	{
		OnReceivedErrorInternal(error);
		return;
	}

	Range data(m_readBuffer, bytesTransferred);
	
	if (global->isServer && !m_socketAuthenticated)
	{
		if (bytesTransferred == (sha256Length + sha256Length))	// Username + auth code
		{
			const Username username = Username(ByteContainer(data.begin(), data.begin() + sha256Length));
			data.Consume(sha256Length);
			
			hash_SHA256 authCode;
			std::copy(data.begin(), data.begin() + sha256Length, authCode.begin());
			data.Consume(sha256Length);
			
			Authenticate(username, authCode);
		}
		else
		{
			LOG << LogSeverity::error << "Authentication message expected but the data received is not of the right size (" << bytesTransferred << ")";
			ErrorDisconnectAll();
		}
	}

	ListenForNewData();
}

void FileStreamConnection::OnReceiveData(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnectionNoTLS> TCPConnectionNoTLS)
{
	if (error)
	{
		OnReceivedErrorInternal(error);
		return;
	}
	
	std::cout << "OnReceiveData " << bytesTransferred << '\n';
	Range data(m_readBuffer, bytesTransferred);

	if (!IsDownloadingFile())
	{
		StartReceiveFile();
	}
	
	if (data.size() != 0)
	{
		ReadChunk(data);
	}
}

void FileStreamConnection::StartReceiveFile()
{
	const auto& fileToReceive = m_filesToReceiveQueue.front();
	m_currentFileDownload = FileReceive();
	
	m_currentFileDownload->fileSize = fileToReceive.fileSize;
	m_currentFileDownload->receivedSize = 0;

	// We create the parent directory if it does not exist
	const std::string filePathFolder = std::filesystem::path(fileToReceive.filePath).parent_path().string();
	if (!filePathFolder.empty() && !std::filesystem::exists(filePathFolder))
	{
		std::filesystem::create_directory(filePathFolder);
	}

	m_currentFileDownload->writeStream = std::ofstream(fileToReceive.filePath, std::ofstream::binary | std::ofstream::app);

	LOG << "Start downloading file " << fileToReceive.fileName << " size: " << fileToReceive.fileSize;
}

void FileStreamConnection::ReadChunk(Range<NetBuffer>& data)
{
	ASSERT(IsDownloadingFile());
	
	m_currentFileDownload->writeStream.write((char*)data.data(), data.size());
	m_currentFileDownload->receivedSize += data.size();
	
	if (m_currentFileDownload->receivedSize >= m_currentFileDownload->fileSize)
	{
		const AttachmentFile& file = m_filesToReceiveQueue.front();

		m_currentFileDownload->writeStream.close();
		LOG << "File " << file.fileName << " download completed";
		
		OnFileDownloadCompleted(file);
		
		m_filesToReceiveQueue.pop_front();
		m_currentFileDownload.reset();

		ListenForNewData();
	}
	else
	{
		const auto bytesLeft = std::min((Int64)m_readBuffer.size(), (m_currentFileDownload->fileSize - m_currentFileDownload->receivedSize));
		ASSERT(bytesLeft > 0);
		
		ListenExactly(bytesLeft);
	}
}

/////////////////////////////// UPLOAD //////////////////////////////////
void FileStreamConnection::SendFileFromQueue()
{
	ASSERT(!m_filesToSendQueue.empty());

	//m_socket->get_io_service().post([this]()
	{
		auto& file = m_filesToSendQueue.front();
		SendFileInternal(file.filePath, file.fileSize);
	}//);
}

void FileStreamConnection::SendFileInternal(const std::string& filePath, UInt64 size)
{
	m_currentFileSend = FileSend();
	m_currentFileSend->fileSize = (UInt32)size;
	m_currentFileSend->sentSize = 0;
	
	m_currentFileSend->readStream = std::ifstream(filePath, std::ios::binary);

	LOG << "Sending file " << filePath << " size: " << m_currentFileSend->fileSize;

	SendChunk();
}

void FileStreamConnection::OnFileSentInternal()
{
	m_currentFileSend->readStream.close();
	m_currentFileSend.reset();
	
	const AttachmentFile fileSent = m_filesToSendQueue.front();
	m_filesToSendQueue.pop_front();
	
	OnFileSentSuccessfully(fileSent);

	if (!m_filesToSendQueue.empty())
		SendFileFromQueue();
}

void FileStreamConnection::SendChunk()
{
	ASSERT(IsSendingFile());

	auto chunk = std::make_shared<WriteBuffer>();
	chunk->chunkSize = std::min((Int64)chunk->data.size(), (Int64)(m_currentFileSend->fileSize - m_currentFileSend->sentSize));

	m_currentFileSend->readStream.seekg(m_currentFileSend->sentSize);
	m_currentFileSend->readStream.read((char*)chunk->data.data(), chunk->chunkSize);
	
	auto self(shared_from_this());
	boost::asio::async_write(*m_socket, boost::asio::buffer(chunk->data, chunk->data.size()), boost::asio::transfer_exactly(chunk->chunkSize), boost::asio::bind_executor(m_strand, [this, self, chunk](boost::system::error_code er, size_t bytesTransferred)
	{
		HandleFileWriteAsync(er, bytesTransferred, chunk, self);
	}));
}

void FileStreamConnection::HandleFileWriteAsync(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<WriteBuffer> chunk, std::shared_ptr<TCPConnectionNoTLS> TCPConnectionNoTLS)
{
	ASSERT(IsSendingFile());
	if (error)
	{
		OnReceivedErrorInternal(error);
		return;
	}

	std::cout << "HandleFileWriteAsync " << bytesTransferred << '\n';
	m_currentFileSend->sentSize += bytesTransferred;

	if (m_currentFileSend->sentSize >= m_currentFileSend->fileSize)
	{
		LOG << "File " << m_filesToSendQueue.front().fileName << " send successfully";
		
		OnFileSentInternal();
	}
	else
	{
		SendChunk();
	}
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