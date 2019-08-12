#pragma once
#include "TCPConnection.h"
#include <fstream>
constexpr auto FILE_STREAM_READ_BUFFER_SIZE = 4096;

class FileStreamConnection : public TCPConnection
{
	struct FileReceive
	{
		static const UInt8 headerLength = 4;
		
		UInt32 fileSize = headerLength;
		UInt32 receivedSize = 0;
		UInt32 writtenSize = 0;

		bool headerReceived = false;
		std::array<byte, headerLength> headerData;
		
		std::ofstream writeStream;
	};

	struct FileSend
	{
		UInt32 fileSize = 0;
		UInt32 sentSize = 0;

		std::ifstream readStream;
	};
	
	std::array<byte, FILE_STREAM_READ_BUFFER_SIZE> m_readBuffer;
	std::array<byte, FILE_STREAM_READ_BUFFER_SIZE> m_writeBuffer;
	std::optional<FileReceive> m_currentFileDownload;
	std::optional<FileSend> m_currentFileSend;
	bool m_socketAuthenticated = false;

public:
	FileStreamConnection(io_context& ioService, ssl::context& context);
	virtual ~FileStreamConnection();

	bool IsDownloadingFile() const;
	bool IsSendingFile() const;
	void SendFile(const std::string& filePath, UInt64 size);

	// TCPConnection
	bool OnConnectedToServerInternal() override;
	bool OnReceivedError(const boost::system::error_code& error) override;

protected:
	virtual void OnFileDownloadCompleted() = 0;
	virtual void ErrorDisconnectAll() = 0;
	virtual void Authenticate(const Username& username, const hash_SHA256& authCode) = 0;
	virtual std::string GetDownloadFilePath(UInt64 fileSize) = 0;
	virtual void OnFileSentSuccessfully() = 0;
	virtual void OnConnectedToServer() = 0;
	
private:
	void StartReading();
	void OnReceiveData(size_t bytesTransferred);

	void OnFullHeaderReceived();
	void OnFilePartReceived(Range<std::array<byte, FILE_STREAM_READ_BUFFER_SIZE>> data);

	void SendFileChunk();
	void HandleFileWriteAsync(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnection> tcpConnection);

	void OnSendingSuccessful(size_t bytesTransferred) override;
};