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
class CommandV2;
class NetworkPayload;
class GenericConnection;

using CommandV2Def = CommandV2;

struct ICommandManager
{
	virtual ~ICommandManager() = default;

	virtual CommandV2 CreateCommand(std::shared_ptr<NetworkPayload> netPayload) = 0;
	virtual CommandV2 CreateCommand(const std::string& cmdName, const DataStructure& data) = 0;
	virtual bool CommandExist(const std::string& cmdShortName) const = 0;
	virtual void OnReceivedCmdInternal(CommandV2& cmd, GenericConnection& connection, void* relatedUser = nullptr) = 0;
	virtual const CommandV2Def& GetCmdDefByFullName(const std::string& name) const = 0;
};