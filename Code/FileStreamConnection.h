// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once
#include <fstream>
#include <deque>
#include "Uuid.h"
#include <boost/asio/strand.hpp>
#include "TCPConnection.h"
constexpr auto FILE_STREAM_BUFFER_SIZE = 1024 * 500;

using NetBuffer = std::array<byte, FILE_STREAM_BUFFER_SIZE>;

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

class FileStreamConnection : public TCPConnection
{
	struct FileReceive
	{
		Int64 fileSize = 0;
		Int64 receivedSize = 0;
		
		std::ofstream writeStream;
	};

	struct FileSend
	{
		Int64 fileSize = 0;
		Int64 sentSize = 0;
		UInt16 packetIndex = 0;

		std::ifstream readStream;
	};
	
	NetBuffer m_readBuffer;
	NetBuffer m_writeBuffer;
	std::optional<FileReceive> m_currentFileDownload;
	std::optional<FileSend> m_currentFileSend;
	bool m_socketAuthenticated = false;

	std::deque<AttachmentFile> m_filesToSendQueue;
	std::deque<AttachmentFile> m_filesToReceiveQueue;
	boost::asio::io_context::strand m_strand;

public:
	FileStreamConnection(io_context& ioService, ssl::context& context);
	virtual ~FileStreamConnection();

	bool IsDownloadingFile() const;
	bool IsSendingFile() const;

	// TCPConnection
	void SetCustomTCPOptions() override;
	bool OnSocketConnectedInternal() override;
	bool OnReceivedError(const boost::system::error_code& error) override;
	void OnSendingSuccessful(size_t bytesTransferred) override {}

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
	void ListenForNewData();
	void ListenExactly(Int64 size);
	void OnReceiveAuthCode(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnection> TCPConnection);
	void OnReceiveData(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnection> TCPConnection);
	void StartReceiveFile();
	void ReadChunk(Range<NetBuffer>& data);

	void SendFileFromQueue();
	void SendFileInternal(const std::string& filePath, UInt64 size);
	void OnFileSentInternal();
	void SendChunk();

	void HandleFileWriteAsync(const boost::system::error_code& error, size_t bytesTransferred, std::shared_ptr<TCPConnection> TCPConnection);
};