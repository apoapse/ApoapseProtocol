#include "stdafx.h"
#include "GenericConnection.h"
#include "Common.h"
#include "MessagePack.hpp"
#include "NetworkPayload.h"

GenericConnection::GenericConnection(boost::asio::io_service& ioService) : TCPConnection(ioService)
{

}

GenericConnection::~GenericConnection()
{
	
}

bool GenericConnection::OnConnectedToServer()
{
	StartReading();
	return true;
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
	auto payload = NetworkPayload(range);
	const size_t currentPayloadLength = (range.Size() > payload.headerInfo.payloadLength) ? payload.headerInfo.payloadLength : range.Size();

	// #TODO check headerInfo.payloadLength for max size allowed
	// #TODO check if command allowed
	payload.payloadData.reserve(payload.headerInfo.payloadLength);
	ProcessPayloadData(range, payload);
}

void GenericConnection::ProcessPayloadData(Range<std::array<byte, READ_BUFFER_SIZE>>& range, NetworkPayload& payload)
{
	const size_t currentPayloadLength = (range.Size() > payload.headerInfo.payloadLength) ? payload.headerInfo.payloadLength : range.Size();

	payload.payloadData.insert(payload.payloadData.end(), range.begin(), range.begin() + currentPayloadLength);
	range.Consume(currentPayloadLength);

	if (payload.BytesLeft() == 0)
	{
		OnReceivedPayload(payload);

		if (range.Size() > 0)
		{
			// This data batch contains data for more than just the current payload
			ProcessHeader(range);
		}
		else
			StartReading();
	}
	else
		ReadSome(m_readBuffer, [this, &payload](size_t bytesTransferred) { OnReceivedPayloadData(bytesTransferred, payload); });
}

void GenericConnection::ReadPayloadData(NetworkPayload& payload)
{
	ReadSome(m_readBuffer, [this, &payload](size_t bytesTransferred) { OnReceivedPayloadData(bytesTransferred, payload); });
}

void GenericConnection::OnReceivedHeaderData(size_t bytesTransferred)
{
	Range <std::array<byte, READ_BUFFER_SIZE>> range(m_readBuffer, bytesTransferred);
	ProcessHeader(range);
}

void GenericConnection::OnReceivedPayloadData(size_t bytesTransferred, NetworkPayload& payload)
{
	Range <std::array<byte, READ_BUFFER_SIZE>> range(m_readBuffer, bytesTransferred);
	ProcessPayloadData(range, payload);
}

void GenericConnection::OnReceivedPayload(NetworkPayload& payload)//TEMP
{
	LOG_DEBUG_FUNCTION_NAME();
	LOG_DEBUG << "payloadLength: " << payload.headerInfo.payloadLength;
}
