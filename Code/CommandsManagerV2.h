#pragma once
#include "ICommandManager.hpp"
#include "CommandV2.h"

using CommandV2Def = CommandV2;

class CommandsManagerV2 : public ICommandManager
{
	std::vector<CommandV2Def> m_registeredCommands;

public:

	CommandsManagerV2(const std::string& cmdSchemeJson);

	CommandV2 CreateCommand(std::shared_ptr<NetworkPayload> netPayload) override;
	CommandV2 CreateCommand(const std::string& cmdName, const DataStructure& data) override;
	bool CommandExist(const std::string& cmdShortName) const override;

private:
	std::optional<CommandV2Def> GetCmdDef(const std::string& shortName);
	const CommandV2Def& GetCmdDefByFullName(const std::string& name) const;

	bool OnReceivedCommandPre(CommandV2& cmd);
	void OnReceivedCommandPost(const CommandV2& cmd);

	bool OnSendCommandPre(CommandV2& cmd);
	bool OnSendCommandPost(const CommandV2& cmd);
};