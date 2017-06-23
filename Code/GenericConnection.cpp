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
	NetworkPayload payload;
	payload.Insert(range);

	if (payload.BytesLeft() == 0)
	{
		OnReceivedPayload(payload);
		StartReading();
	}
	else
	{
		ReadPayloadData(payload);
	}
}

void GenericConnection::ProcessPayloadData(Range<std::array<byte, READ_BUFFER_SIZE>>& range, NetworkPayload& payload)
{
	payload.Insert(range);

	if (payload.BytesLeft() == 0)
	{
		if (range.Size() == 0)
		{
			OnReceivedPayload(payload);
			StartReading();
		}
		else
		{
			NetworkPayload payload;
			payload.Insert(range);

			ProcessPayloadData(range, payload);
		}
	}
	else
		ReadPayloadData(payload);
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
	ASSERT(payload.headerInfo->payloadLength == payload.payloadData.size());

	LOG_DEBUG_FUNCTION_NAME();
	LOG_DEBUG << "payloadLength: " << payload.headerInfo->payloadLength;
}
