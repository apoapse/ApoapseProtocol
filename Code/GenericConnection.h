#pragma once
#include "TCPConnection.h"
#include "ReadBufferSize.hpp"
#include "Range.hpp"
#include <memory>
#include "Commands.hpp"
#include <chrono>
#include "Command.h"
class NetworkPayload;

class GenericConnection : public TCPConnection
{
	std::array<byte, READ_BUFFER_SIZE> m_readBuffer; // #THREADING
	std::chrono::steady_clock::time_point m_lastActivityTime;// #TODO USE #THREADING
public:
	GenericConnection(boost::asio::io_service& ioService);
	virtual ~GenericConnection() override = default;
	
private:
	bool OnConnectedToServerInternal() override;
	virtual bool OnReceivedError(const boost::system::error_code& error) override;
	void StartReading();
	void ProcessHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& range, size_t bytesTransferred);
	void ProcessDataGeneric(Range<std::array<byte, READ_BUFFER_SIZE>>& range, std::shared_ptr<NetworkPayload> payload, size_t bytesTransferred);
	void ReadPayloadData(std::shared_ptr<NetworkPayload> payload);
	void OnReceivedHeaderData(size_t bytesTransferred);
	void OnReceivedPayloadData(size_t bytesTransferred, std::shared_ptr<NetworkPayload> payload);
	void OnReceivedPayload(std::shared_ptr<NetworkPayload> payload);
	virtual void OnSendingSuccessful(size_t bytesTransferred) override;
	void UpdateLastActivityClock();

protected:
	virtual bool OnConnectedToServer() = 0;
	virtual void OnReceivedValidCommand(std::unique_ptr<Command> cmd) = 0;
	//virtual bool CanProcessCommandFromThisConnection(CommandId command) = 0;
};