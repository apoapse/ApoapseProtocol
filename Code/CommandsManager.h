#pragma once
#include <string>
#include <memory>
#include "ObjectsFactory.hpp"
#include "Command.h"
#include "TypeDefs.hpp"

#define APOAPSE_COMMAND_REGISTER(_class, _key)	OBJECTS_FACTORY_REGISTER(CommandsManager::GetInstance().commandsFactory, Command, _class, CommandId, _key)

class CommandsManager
{
	CommandsManager() = default;

public:
	ObjectsFactory<CommandId, Command> commandsFactory;

	CommandsManager(CommandsManager const&) = delete;
	void operator=(CommandsManager const&) = delete;

	static CommandsManager& GetInstance()
	{
		static CommandsManager commandsManager;
		return commandsManager;
	}

	std::unique_ptr<Command> CreateCommand(CommandId commandName);
	bool CommandExist(CommandId commandName) const;
};