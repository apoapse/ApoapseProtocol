#include "stdafx.h"
#include "Common.h"
#include "CommandsManagerV2.h"
#include "Json.hpp"
#include "NetworkPayload.h"
#include "SecurityAlert.h"
#include "GenericConnection.h"
#include "ApoapseOperation.h"
#include "StringExtensions.h"

CommandsManagerV2::CommandsManagerV2(const std::string& cmdSchemeJson)
{
	JsonHelper json(cmdSchemeJson);

	for (const auto& dser : json.ReadFieldArray<JsonHelper>("commands"))
	{
		CommandV2Def cmd;
		cmd.name = dser.ReadFieldValue<std::string>("name").value();
		cmd.nameShort = dser.ReadFieldValue<std::string>("name_short").value();
		cmd.relatedDataStructure = dser.ReadFieldValue<std::string>("datastructure").value();
		cmd.requireAuthentication = dser.ReadFieldValue<bool>("require_authentication").value();
		cmd.onlyNonAuthenticated = dser.ReadFieldValue<bool>("only_non_authenticated").value_or(false);
		cmd.clientUIPropagate = dser.ReadFieldValue<bool>("client_ui.propagate").value_or(false);
		cmd.clientUISignalName = dser.ReadFieldValue<std::string>("client_ui.signal_name").value_or(std::string());
		cmd.operationRegister = dser.ReadFieldValue<bool>("operation.register").value_or(false);
		cmd.operationOwnership = CommandV2::ConvertFieldToOwnership(dser.ReadFieldValue<std::string>("operation.ownership").value_or(""));
		cmd.saveOnReceive = dser.ReadFieldValue<bool>("save_on_receive").value_or(false);
		cmd.receiveOnClient = dser.ReadFieldValue<bool>("reception.client").value_or(false);
		cmd.receiveOnServer = dser.ReadFieldValue<bool>("reception.server").value_or(false);
		cmd.propagate = dser.ReadFieldValue<bool>("propagation.propagate").value_or(false);
		cmd.excludeSelfPropagation = dser.ReadFieldValue<bool>("propagation.exclude_self").value_or(false);

		if (dser.ReadFieldValue<std::string>("required_permissions").is_initialized())
		{
			const std::string permStr = dser.ReadFieldValue<std::string>("required_permissions").value();
			if (!permStr.empty())
			{
				StringExtensions::split(permStr, cmd.requiredPermissions, " ");
			}
		}

		if (!GetCmdDef(cmd.nameShort).has_value())
		{
			m_registeredCommands.push_back(cmd);
		}
		else
		{
			LOG << LogSeverity::error << "Command: " << cmd.name << " The short name " << cmd.nameShort << " is already used by the command named " << GetCmdDef(cmd.nameShort)->name;
		}
	}

	LOG << "Registered " << m_registeredCommands.size() << " commands";
}

CommandV2 CommandsManagerV2::CreateCommand(std::shared_ptr<NetworkPayload> netPayload)
{
	auto cmdDef = GetCmdDef(netPayload->headerInfo->cmdShortName);
	if (!cmdDef)
	{
		throw std::exception("The related command do not exist.");
	}

	CommandV2 cmd = cmdDef.value();
	auto data = global->apoapseData->FromNetwork(cmd, netPayload);

	cmd.SetData(data);

	return cmd;
}

CommandV2 CommandsManagerV2::CreateCommand(const std::string& cmdName, const DataStructure& data)
{
	CommandV2 cmd = GetCmdDefByFullName(cmdName);
	cmd.SetData(data);

	return cmd;
}

bool CommandsManagerV2::CommandExist(const std::string& cmdShortName) const
{
	return std::count_if(m_registeredCommands.begin(), m_registeredCommands.end(), [&cmdShortName](const CommandV2Def& cmd)
	{
		return (cmd.nameShort == cmdShortName);
	});
}

void CommandsManagerV2::OnReceivedCmdInternal(CommandV2& cmd, GenericConnection& connection, void* relatedUser)
{
	if (OnReceivedCommandPre(cmd, connection))
	{
		// Cmd auto-save
		if (cmd.saveOnReceive)
			cmd.GetData().SaveToDatabase();

		// Cmd process
		OnReceivedCommand(cmd, connection);

		// Cmd receive client notify UI
		if (global->isClient && cmd.clientUIPropagate)
			PropagateToClientUI(cmd);

		// Cmd server propagation to requested connected clients
		if (global->isServer && cmd.propagate)
			Propagate(cmd, connection);

		// Operation registration
		if (cmd.operationRegister)
			ApoapseOperation::RegisterOperation(cmd, connection.GetConnectedUser());

		//if (global->isServer && cmd.propagateToOtherClients)
		//	cmd.Send(*connection.server.usersManager, &netConnection);
	}
	else
	{
		LOG << LogSeverity::error << "The command " << cmd.name << " was rejected by CommandsManagerV2::OnReceivedCommandPre";
		SecurityLog::LogAlert(ApoapseErrorCode::invalid_cmd, connection);
	}
}

void CommandsManagerV2::PropagateToClientUI(CommandV2& cmd) const
{
	cmd.GetData().SendUISignal(cmd.clientUISignalName);
}

std::optional<CommandV2Def> CommandsManagerV2::GetCmdDef(const std::string& shortName)
{
	const auto& res = std::find_if(m_registeredCommands.begin(), m_registeredCommands.end(), [&shortName](const CommandV2Def& cmd)
	{
		return (cmd.nameShort == shortName);
	});

	if (res != m_registeredCommands.end())
		return *res;

	return std::optional<CommandV2Def>();
}

const CommandV2Def& CommandsManagerV2::GetCmdDefByFullName(const std::string& name) const
{
	const auto& res = std::find_if(m_registeredCommands.begin(), m_registeredCommands.end(), [&name](const CommandV2Def& cmd)
		{
			return (cmd.name == name);
		});

	if (res == m_registeredCommands.end())
		throw std::exception("The requested command do not exist. Wrong full name.");

	return *res;
}
