#pragma once
//#include "TCPConnection.h"
#include <fstream>
#include <deque>
#include "Uuid.h"
#include <boost/asio/strand.hpp>
#include <mutex>
#include "TCPConnectionNoTLS.h"
constexpr auto FILE_STREAM_READ_BUFFER_SIZE = 1024 * 500;

struct AttachmentFile
{
	Uuid uuid;
	Uuid relatedMessage;
	std::string filePath;
	std::string fileName;
	size_t fileSize = 0;

	AttachmentFile() = default;
	AttachmentFile(DataStructure& data, const std::string& filePath);
};

class FileStreamConnection : public TCPConnectionNoTLS
{
	struct FileReceive
	{
		UInt32 fileSize = 0;
		UInt32 receivedSize = 0;
		UInt32 writtenSize = 0;
		
		std::ofstream writeStream;
	};

	struct FileSend
	{
		UInt32 fileSize = 0;
		UInt32 sentSize = 0;
		UInt16 packetIndex = 0;

		std::ifstream readStream;
	};
	
	std::array<byte, FILE_STREAM_READ_BUFFER_SIZE> m_readBuffer;
	std::array<byte, FILE_STREAM_READ_BUFFER_SIZE> m_writeBuffer;
	std::optional<FileReceive> m_currentFileDownload;
	std::optional<FileSend> m_currentFileSend;
	bool m_socketAuthenticated = false;

	std::deque<AttachmentFile> m_filesToSendQueue;
	std::deque<AttachmentFile> m_filesToReceiveQueue;
	boost::asio::io_context::strand m_strand;
	std::vector<UInt32> m_bytesWriten;

public:
	FileStreamConnection(io_context& ioService/*, ssl::context& context*/);
	virtual ~FileStreamConnection();

	bool IsDownloadingFile() const;
	bool IsSendingFile() const;

	// TCPConnection
	bool OnSocketConnectedInternal() override;
	bool OnReceivedError(const boost::system::error_code& error) override;

	void PushFileToReceive(const AttachmentFile& file);
	void PushFileToSend(const AttachmentFile& file);

	void SetAuthenticated();

protected:
	virtual void OnFileDownloadCompleted(const AttachmentFile& file) = 0;
	virtual void ErrorDisconnectAll() = 0;
	virtual void Authenticate(const Username&/* username*/, const hash_SHA256&/* authCode*/) { ASSERT(false); }
	virtual void OnFileSentSuccessfully(const AttachmentFile& file) = 0;
	virtual void OnSocketConnected() = 0;
	
private:
	void SendFileFromQueue();
	void SendFileInternal(const std::string& filePath, UInt64 size);
	void OnFileSentInternal();
	
	void StartReading();
	void OnReceiveData(size_t bytesTransferred, std::shared_ptr<TCPConnectionNoTLS> tcpConnection);
	UInt16 ReadFromFile();

	void OnFilePartReceived(Range<std::array<byte, FILE_STREAM_READ_BUFFER_SIZE>> data);

	void HandleFileWriteAsync(const boost::system::error_code& error, size_t bytesTransferred, UInt16 packetIndex, std::shared_ptr<TCPConnectionNoTLS> tcpConnection);

	void OnSendingSuccessful(size_t bytesTransferred) override;	// Used with generic ByteContainer, strings sends not for files
};