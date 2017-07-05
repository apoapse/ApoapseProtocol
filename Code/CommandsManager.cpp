#include "stdafx.h"
#include "CommandsManager.h"
#include "Common.h"
#include "Command.h"

std::unique_ptr<Command> CommandsManager::CreateCommand(Commands commandName)
{
	ASSERT_MSG(CommandExist(commandName), "The command requested does not exist");

	return commandsFactory.CreateObject(commandName);
}

bool CommandsManager::CommandExist(Commands commandName) const
{
	return commandsFactory.IsObjectExist(commandName);
}