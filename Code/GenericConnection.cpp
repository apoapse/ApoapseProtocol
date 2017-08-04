#include "stdafx.h"
#include "GenericConnection.h"
#include "Common.h"
#include "MessagePack.hpp"
#include "NetworkPayload.h"

GenericConnection::GenericConnection(boost::asio::io_service& ioService) : TCPConnection(ioService)
{

}

void GenericConnection::Send(Commands command, std::shared_ptr<std::vector<byte>> data, TCPConnection* excludedConnection)
{
	if (excludedConnection == this)
		return;

	{
		auto header = std::make_shared<std::vector<byte>>(NetworkPayload::GenerateHeader(command, *data));	// #TODO #OPTIMIZATION Find a way to use a unique_ptr instead, the blocker to that are the boost::bind in TCPConnection
		TCPConnection::Send(header);
	}

	TCPConnection::Send(data);
}

bool GenericConnection::OnConnectedToServerInternal()
{
	StartReading();

	return OnConnectedToServer();
}

bool GenericConnection::OnReceivedError(const boost::system::error_code& error)
{
	LOG << error.message() << LogSeverity::warning;

	return true;
}

void GenericConnection::StartReading()
{
	ReadSome(m_readBuffer, [this](size_t bytesTransferred) { OnReceivedHeaderData(bytesTransferred); });
}

void GenericConnection::ProcessHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& range)
{
	auto payload = std::make_shared<NetworkPayload>();
	ProcessDataGeneric(range, std::move(payload));
}

void GenericConnection::ProcessDataGeneric(Range<std::array<byte, READ_BUFFER_SIZE>>& range, std::shared_ptr<NetworkPayload> payload)
{
	try
	{
		if (range.size() > 0)
			payload->Insert(range);
	
		if (payload->BytesLeft() == 0)
		{
			if (range.size() == 0)
			{
				OnReceivedPayloadInternal(std::move(payload));
				StartReading();
			}
			else
			{
				OnReceivedPayloadInternal(std::move(payload));
	
				// The buffer has data for more than one network payload
				auto payloadLocal = std::make_shared<NetworkPayload>();
				payloadLocal->Insert(range);
	
				ProcessDataGeneric(range, std::move(payloadLocal));
			}
		}
		else
			ReadPayloadData(std::move(payload));
	}
	catch (const std::exception& e)
	{
		LOG << e << LogSeverity::error;
		Close();
	}
}

void GenericConnection::ReadPayloadData(std::shared_ptr<NetworkPayload> payload)
{
	ReadSome(m_readBuffer, [this, payload](size_t bytesTransferred) { OnReceivedPayloadData(bytesTransferred, payload); });
}

void GenericConnection::OnReceivedHeaderData(size_t bytesTransferred)
{
	UpdateLastActivityClock();

	Range <std::array<byte, READ_BUFFER_SIZE>> range(m_readBuffer, bytesTransferred);
	ProcessHeader(range);
}

void GenericConnection::OnReceivedPayloadData(size_t bytesTransferred, std::shared_ptr<NetworkPayload> payload)
{
	UpdateLastActivityClock();

	Range <std::array<byte, READ_BUFFER_SIZE>> range(m_readBuffer, bytesTransferred);
	ProcessDataGeneric(range, std::move(payload));
}

void GenericConnection::OnReceivedPayloadInternal(std::shared_ptr<NetworkPayload> payload)
{
	ASSERT(payload->headerInfo->payloadLength == payload->payloadData.size());

	LOG_DEBUG_FUNCTION_NAME();
	LOG_DEBUG << "payloadLength: " << payload->headerInfo->payloadLength;

	OnReceivedPayload(std::move(payload));
}

void GenericConnection::OnSendingSuccessful(size_t bytesTransferred)
{
	UpdateLastActivityClock();
}

void GenericConnection::UpdateLastActivityClock()
{
	m_lastActivityTime = std::chrono::steady_clock::now();
}
