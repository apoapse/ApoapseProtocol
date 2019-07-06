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

bool CommandV2::IsValid() const
{
	return m_data.isValid;
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
