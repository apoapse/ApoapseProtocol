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

#pragma once
#include "ICommandManager.hpp"
#include "CommandV2.h"

class CommandsManagerV2 : public ICommandManager
{
	friend class CommandV2;
	std::vector<CommandV2Def> m_registeredCommands;

public:

	CommandsManagerV2(const std::string& cmdSchemeJson);

	CommandV2 CreateCommand(std::shared_ptr<NetworkPayload> netPayload) override;
	CommandV2 CreateCommand(const std::string& cmdName, const DataStructure& data) override;
	bool CommandExist(const std::string& cmdShortName) const override;
	const CommandV2Def& GetCmdDefByFullName(const std::string& name) const override;

	void OnReceivedCmdInternal(CommandV2& cmd, GenericConnection& connection, void* relatedUser = nullptr) override;

	void PropagateToClientUI(CommandV2& cmd) const;

protected:
	std::optional<CommandV2Def> GetCmdDef(const std::string& shortName);

private:
	virtual bool OnReceivedCommandPre(CommandV2& cmd, GenericConnection& netConnection) = 0;
	virtual void OnReceivedCommand(CommandV2& cmd, GenericConnection& netConnection) = 0;
	virtual void OnReceivedCommandPost(CommandV2& cmd, GenericConnection& netConnection) = 0;
	virtual bool OnSendCommandPre(CommandV2&/* cmd*/) { return true; }
	virtual void Propagate(CommandV2& /*cmd*/, GenericConnection& /*netConnection*/) {};

	/*bool OnSendCommandPre(CommandV2& cmd);
	bool OnSendCommandPost(const CommandV2& cmd);*/
};