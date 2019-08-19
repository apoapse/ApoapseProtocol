#include "stdafx.h"
#include "Common.h"
#include "CommandV2.h"
#include "NetworkPayload.h"
#include "CommandsManagerV2.h"
#include "ApoapseOperation.h"
#include "UsergroupManager.h"

CommandV2::~CommandV2()
{
	//LOG_DEBUG << "~CommandV2";
}

CommandV2::CommandV2(DataStructure& data) : m_data(data)
{
}

OperationOwnership CommandV2::ConvertFieldToOwnership(const std::string& value)
{
	if (value == "self")
		return OperationOwnership::self;

	else if (value == "usergroup")
		return OperationOwnership::usergroup;

	else if (value == "all")
		return OperationOwnership::all;

	else
		return OperationOwnership::undefined;
}

void CommandV2::SetData(const DataStructure& data)
{
	m_data = data;
}

bool CommandV2::IsValid(const IUser* user) const
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

	if (onlyNonAuthenticated && user)
	{
		LOG << LogSeverity::warning << "Command " << name << ": this command is only when the user is not authenticated";
		return false;
	}

	if (requireAuthentication && !user)
	{
		LOG << LogSeverity::warning << "Command " << name << ": this command require the user to be authenticated";
		return false;
	}

	if (requireAuthentication && !user)
	{
		LOG << LogSeverity::warning << "Command " << name << ": this command require the user to be authenticated";
		return false;
	}

	if (global->isServer)
	{
		if (onlyTemporaryAuth && user && !user->IsUsingTemporaryPassword())
		{
			LOG << LogSeverity::warning << "Command " << name << ": this command require the user to be authenticated but with a temporary password";
			return false;
		}

		if (requireAuthentication && !onlyTemporaryAuth && user->IsUsingTemporaryPassword())
		{
			LOG << LogSeverity::warning << "Command " << name << ": this command require the user to be authenticated without a temporary password";
			return false;
		}
	}

	if (global->isServer && !requiredPermissions.empty())
	{
		for (const auto& permission : requiredPermissions)
		{
			if (!user->GetUsergroup().HasPermission(permission))
			{
				LOG << LogSeverity::warning << "The user sent the command " << name << " but he does not have the required permission " << permission;
				return false;
			}
		}
	}

	return true;
}

void CommandV2::Send(INetworkSender& destination, TCPConnection* excludedConnection)
{
	if (operationRegister)
	{
		ApoapseOperation::PrepareCmdSend(*this);
	}

	m_data.AutoFillFieldsIfRequested();

	if (!static_cast<CommandsManagerV2*>(global->cmdManager.get())->OnSendCommandPre(*this))
	{
		LOG << LogSeverity::error << "The command send was stopped by CommandsManagerV2::OnSendCommandPre";
		return;
	}

	auto msgPack = m_data.GetMessagePackFormat();
	const auto& msgPackBytes = msgPack.GetMessagePackBytes();
	auto bytes = std::vector<byte>(NetworkPayload::headerLength + msgPackBytes.size());

	std::copy(msgPackBytes.begin(), msgPackBytes.end(), bytes.begin() + NetworkPayload::headerLength);
	
	LOG_DEBUG << "Sending command " << name;
	
	auto payload = std::make_shared<NetworkPayload>(nameShort, std::move(bytes));
	destination.Send(payload, excludedConnection);
}

DataStructure& CommandV2::GetData()
{
	return m_data;
}