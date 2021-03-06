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
#include "Common.h"
#include "CommandsManagerV2.h"
#include "Json.hpp"
#include "NetworkPayload.h"
#include "GenericConnection.h"
#include "ApoapseOperation.h"
#include "StringExtensions.h"
#include "ApoapseError.h"

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
		cmd.onlyTemporaryAuth = dser.ReadFieldValue<bool>("only_temporary_auth").value_or(false);
		cmd.clientUIPropagate = dser.ReadFieldValue<bool>("client_ui.propagate").value_or(false);
		cmd.clientUISignalName = dser.ReadFieldValue<std::string>("client_ui.signal_name").value_or(std::string());
		cmd.operationRegister = dser.ReadFieldValue<bool>("operation.register").value_or(false);
		cmd.operationDatastructure = dser.ReadFieldValue<std::string>("operation.datastructure").value_or(cmd.relatedDataStructure);
		cmd.operationOwnership = CommandV2::ConvertFieldToOwnership(dser.ReadFieldValue<std::string>("operation.ownership").value_or(""));
		cmd.saveOnReceiveServer = dser.ReadFieldValue<bool>("save_on_receive.server").value_or(false);
		cmd.saveOnReceiveClient = dser.ReadFieldValue<bool>("save_on_receive.client").value_or(false);
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
		throw std::runtime_error("The related command do not exist.");
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
		if ((global->isServer && cmd.saveOnReceiveServer) || (global->isClient && cmd.saveOnReceiveClient))
			cmd.GetData().SaveToDatabase();

		// Cmd process
		OnReceivedCommand(cmd, connection);

		if (!connection.m_isConnected)	// In case a Close was called on the previous cmd pipeline step
		{
			LOG << "Command processing cancelled prematurely because a Close have been called on the connection";
			return;
		}

		// Cmd receive client notify UI
		if (global->isClient && cmd.clientUIPropagate)
			PropagateToClientUI(cmd);

		// Cmd server propagation to requested connected clients
		if (global->isServer && cmd.propagate)
			Propagate(cmd, connection);

		// Operation registration
		if (cmd.operationRegister)
			ApoapseOperation::RegisterOperation(cmd, connection.GetConnectedUser());

		// Cmd post process
		OnReceivedCommandPost(cmd, connection);	}
	else
	{
		LOG << LogSeverity::error << "The command " << cmd.name << " was rejected by CommandsManagerV2::OnReceivedCommandPre";
		
		if (!connection.CloseRequested())
			ApoapseError(ApoapseErrors::invalid_cmd, &connection);
	}

	if (connection.CloseRequested())
		connection.Close();
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
		throw std::runtime_error("The requested command do not exist. Wrong full name.");

	return *res;
}
