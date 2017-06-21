#pragma once
#include "TCPConnection.h"
#include "ReadBufferSize.hpp"

class GenericConnection : public TCPConnection
{
	std::array<byte, READ_BUFFER_SIZE> m_readBuffer;

public:
	GenericConnection(boost::asio::io_service& ioService);
	virtual ~GenericConnection();
	
private:
	virtual bool OnConnectedToServer() override;
	virtual bool OnReceivedError(const boost::system::error_code& error) override;
	void StartReading();
	void OnReceivedHeaderData(size_t bytesTransferred);
};