#pragma once
#include "TCPConnection.h"
#include "ReadBufferSize.hpp"
#include "Range.hpp"
struct NetworkPayload;

class GenericConnection : public TCPConnection
{
	std::array<byte, READ_BUFFER_SIZE> m_readBuffer;
	//TODO: last activity counter
public:
	GenericConnection(boost::asio::io_service& ioService);
	virtual ~GenericConnection();
	
private:
	bool OnConnectedToServer() override;
	bool OnReceivedError(const boost::system::error_code& error) override;
	void StartReading();
	void ProcessHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& range);
	void ProcessPayloadData(Range<std::array<byte, READ_BUFFER_SIZE>>& range, NetworkPayload& payload);
	void ReadPayloadData(NetworkPayload& payload);
	void OnReceivedHeaderData(size_t bytesTransferred);
	void OnReceivedPayloadData(size_t bytesTransferred, NetworkPayload& payload);

protected:
	virtual void OnReceivedPayload(NetworkPayload& payload)/* = 0*/;
};