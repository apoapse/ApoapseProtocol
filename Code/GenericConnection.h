#pragma once
#include "TCPConnection.h"
#include "ReadBufferSize.hpp"
#include "Range.hpp"
#include <memory>
#include "IdsCommand.hpp"
struct NetworkPayload;

class GenericConnection : public TCPConnection
{
	std::array<byte, READ_BUFFER_SIZE> m_readBuffer;
	//TODO: last activity counter
public:
	GenericConnection(boost::asio::io_service& ioService);
	virtual ~GenericConnection();

	void Send(IdsCommand command, std::shared_ptr<std::vector<byte>> data, TCPConnection* excludedConnection);
	
private:
	bool OnConnectedToServer() override;
	bool OnReceivedError(const boost::system::error_code& error) override;
	void StartReading();
	void ProcessHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& range);
	void ProcessDataGeneric(Range<std::array<byte, READ_BUFFER_SIZE>>& range, std::shared_ptr<NetworkPayload> payload);
	void ReadPayloadData(std::shared_ptr<NetworkPayload> payload);
	void OnReceivedHeaderData(size_t bytesTransferred);
	void OnReceivedPayloadData(size_t bytesTransferred, std::shared_ptr<NetworkPayload> payload);

protected:
	virtual void OnReceivedPayloadInternal(std::shared_ptr<NetworkPayload> payload);
};