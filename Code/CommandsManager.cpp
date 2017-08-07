#include "stdafx.h"
#include "CommandsManager.h"
#include "Common.h"
#include "Command.h"

std::unique_ptr<Command> CommandsManager::CreateCommand(CommandId commandName)
{
	ASSERT_MSG(CommandExist(commandName), "The command requested does not exist");

	return commandsFactory.CreateObject(commandName);
}

bool CommandsManager::CommandExist(CommandId commandName) const
{
	return commandsFactory.DoesObjectExist(commandName);
}