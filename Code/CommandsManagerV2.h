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

	void OnReceivedCmdInternal(CommandV2& cmd, GenericConnection& connection, void* relatedUser = nullptr) override;

	void PropagateToClientUI(CommandV2& cmd) const;

private:
	std::optional<CommandV2Def> GetCmdDef(const std::string& shortName);
	const CommandV2Def& GetCmdDefByFullName(const std::string& name) const;

	virtual bool OnReceivedCommandPre(CommandV2& cmd, GenericConnection& netConnection) = 0;
	virtual void OnReceivedCommand(CommandV2& cmd, GenericConnection& netConnection) = 0;

	/*bool OnSendCommandPre(CommandV2& cmd);
	bool OnSendCommandPost(const CommandV2& cmd);*/
};