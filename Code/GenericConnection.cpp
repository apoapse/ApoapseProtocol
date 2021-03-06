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

#include "stdafx.h"
#include "GenericConnection.h"
#include "Common.h"
#include "MessagePack.hpp"
#include "NetworkPayload.h"
#include "ApoapseError.h"
#include "CommandV2.h"

GenericConnection::GenericConnection(boost::asio::io_service& ioService, ssl::context& context) : TCPConnection(ioService, context)
{

}

bool GenericConnection::CloseRequested() const
{
	return m_closeRequested;
}

bool GenericConnection::OnSocketConnectedInternal()
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
#ifndef DEBUG
	try
	{
#endif // !DEBUG
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

#ifndef DEBUG
	}
	catch (const std::length_error&)
	{
		LOG << "length_error" << LogSeverity::error;
		Close();
	}
	catch (const std::exception& e)
	{
		LOG << e << LogSeverity::error;
		Close();
	}
#endif // !DEBUG
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

void GenericConnection::OnReceivedPayload(const std::shared_ptr<NetworkPayload>& payload)
{
	if (!global->cmdManager->CommandExist(payload->headerInfo->cmdShortName))
	{
		ApoapseError(ApoapseErrors::unknown_cmd, this);
		return;
	}

	auto cmd = std::make_shared<CommandV2>(global->cmdManager->CreateCommand(payload));
	LOG_DEBUG << "Received command " << cmd->name << " from payload. Total size: " << payload->GetRawData().size();

#ifndef DEBUG
	try
	{
#endif
		OnReceivedCommand(*cmd.get());
#ifndef DEBUG
	}
	catch (const std::exception& e)
	{
		LOG << LogSeverity::error << "Exception trigged while processing a command of type " << cmd->name << ": " << e;
		Close();
	}
#endif
}

void GenericConnection::OnSendingSuccessful(size_t bytesTransferred)
{
	UpdateLastActivityClock();
}

void GenericConnection::UpdateLastActivityClock()
{
	m_lastActivityTime = std::chrono::steady_clock::now();
}

void GenericConnection::SetCustomTCPOptions()
{
	const boost::asio::socket_base::reuse_address option(true);
	GetSocket().set_option(option);
}
