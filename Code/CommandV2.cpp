#include "stdafx.h"
#include "Common.h"
#include "CommandV2.h"
#include "NetworkPayload.h"

CommandV2::CommandV2(DataStructure& data) : m_data(data)
{
}

void CommandV2::SetData(const DataStructure& data)
{
	m_data = data;
}

bool CommandV2::IsValid(bool isAuthenticated) const
{
	if (!m_data.isValid)
	{
		LOG << LogSeverity::warning << "Command " << name << ": data invalid";
		return false;
	}

	if (global->isClient && !receiveOnClient)
	{
		LOG << LogSeverity::warning << "Command " << name << ": this command cannot be received on a client";
		return false;
	}

	if (global->isServer && !receiveOnServer)
	{
		LOG << LogSeverity::warning << "Command " << name << ": this command cannot be received on a server";
		return false;
	}

	if (onlyNonAuthenticated && isAuthenticated)
	{
		LOG << LogSeverity::warning << "Command " << name << ": this command is only when the user is not authenticated";
		return false;
	}

	if (requireAuthentication && !isAuthenticated)
	{
		LOG << LogSeverity::warning << "Command " << name << ": this command require the user to be authenticated";
		return false;
	}

	return true;
}

void CommandV2::Send(INetworkSender& destination, TCPConnection* excludedConnection)
{
	auto msgPack = m_data.GetMessagePackFormat();
	const auto& msgPackBytes = msgPack.GetMessagePackBytes();
	auto bytes = std::vector<byte>(NetworkPayload::headerLength + msgPackBytes.size());

	std::copy(msgPackBytes.begin(), msgPackBytes.end(), bytes.begin() + NetworkPayload::headerLength);

	auto payload = std::make_shared<NetworkPayload>(nameShort, std::move(bytes));
	destination.Send(payload, excludedConnection);
}

DataStructure& CommandV2::GetData()
{
	return m_data;
}