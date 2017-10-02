#include "stdafx.h"
#include "GenericConnection.h"
#include "Common.h"
#include "MessagePack.hpp"
#include "NetworkPayload.h"
#include "SecurityAlert.h"
#include "CommandsManager.h"

GenericConnection::GenericConnection(boost::asio::io_service& ioService) : TCPConnection(ioService)
{

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

void GenericConnection::ProcessHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& range, size_t bytesTransferred)
{
	auto payload = std::make_shared<NetworkPayload>();
	ProcessDataGeneric(range, std::move(payload), bytesTransferred);
}

void GenericConnection::ProcessDataGeneric(Range<std::array<byte, READ_BUFFER_SIZE>>& range, std::shared_ptr<NetworkPayload> payload, size_t bytesTransferred)
{
	try
	{
		if (range.size() > 0)
			payload->Insert(range, bytesTransferred);

		if (payload->BytesLeft() == 0)
		{
			if (range.size() == 0)
			{
				OnReceivedPayload(std::move(payload));
				StartReading();
			}
			else
			{
				OnReceivedPayload(std::move(payload));
	
				// The buffer has data for more than one network payload, so we create a new one
				auto payloadLocal = std::make_shared<NetworkPayload>();
				//payloadLocal->Insert(range, bytesTransferred);
	
				ProcessDataGeneric(range, std::move(payloadLocal), bytesTransferred);
			}
		}
		else
			ReadPayloadData(std::move(payload));
	}
	catch (const std::length_error&)
	{
		SecurityLog::LogAlert(ApoapseErrorCode::network_message_too_long, *this);
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
	ProcessHeader(range, bytesTransferred);
}

void GenericConnection::OnReceivedPayloadData(size_t bytesTransferred, std::shared_ptr<NetworkPayload> payload)
{
	UpdateLastActivityClock();

	Range <std::array<byte, READ_BUFFER_SIZE>> range(m_readBuffer, bytesTransferred);
	ProcessDataGeneric(range, std::move(payload), bytesTransferred);
}

void GenericConnection::OnReceivedPayload(std::shared_ptr<NetworkPayload> payload)
{
// 	if (payload->headerInfo->payloadLength != payload->m_rawPayloadData.size())
// 	{
// 		SecurityLog::LogAlert(ApoapseErrorCode::wrong_network_payload_length, *this);
// 		return;
// 	}

	if (!CommandsManager::GetInstance().CommandExist(payload->headerInfo->command))
	{
		SecurityLog::LogAlert(ApoapseErrorCode::unknown_cmd, *this);
		return;
	}

	auto cmd = CommandsManager::GetInstance().CreateCommand(payload->headerInfo->command);
	LOG_DEBUG << "Received command " << static_cast<UInt16>(cmd->GetInfo().command) << " from payload total size: " << payload->GetRawData().size();
	cmd->Parse(payload);

	if (cmd->IsValid())
		OnReceivedValidCommand(std::move(cmd));
	else
		SecurityLog::LogAlert(ApoapseErrorCode::invalid_cmd, *this);
}

void GenericConnection::OnSendingSuccessful(size_t bytesTransferred)
{
	UpdateLastActivityClock();
}

void GenericConnection::UpdateLastActivityClock()
{
	m_lastActivityTime = std::chrono::steady_clock::now();
}
