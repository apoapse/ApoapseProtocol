#include "stdafx.h"
#include "Common.h"
#include "CommandsManagerV2.h"
#include "Json.hpp"
#include "NetworkPayload.h"
#include "SecurityAlert.h"
#include "GenericConnection.h"

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
		cmd.propagateToOtherClients = dser.ReadFieldValue<bool>("propagate_to_other_clients").value_or(false);
		cmd.clientUIPropagate = dser.ReadFieldValue<bool>("client_ui.propagate").value_or(false);
		cmd.clientUISignalName = dser.ReadFieldValue<std::string>("client_ui.signal_name").value_or(std::string());
		cmd.operationRegister = dser.ReadFieldValue<bool>("operation.register").value_or(false);
		cmd.operationOwnership = (OperationOwnership)dser.ReadFieldValue<int>("operation.ownership").value_or(0);
		cmd.receiveOnClient = dser.ReadFieldValue<bool>("reception.client").value_or(false);
		cmd.receiveOnServer = dser.ReadFieldValue<bool>("reception.server").value_or(false);

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
	auto data = global->apoapseData->FromNetwork(cmdDef->relatedDataStructure, netPayload);

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
		OnReceivedCommand(cmd, connection);

		if (cmd.clientUIPropagate)
			PropagateToClientUI(cmd);

		// TODO2: Add auto save, net propagation and operation registration
	}
	else
	{
		LOG << LogSeverity::error << "The command " << cmd.name << " was rejected by the code";
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
