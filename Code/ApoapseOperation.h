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
#include "TypeDefs.hpp"
#include "Username.h"
#include "CommandV2.h"
class CommandV2;
class GenericConnection;

class ApoapseOperation
{
public:
	static void RegisterOperation(CommandV2& cmd, const std::optional<Username>& sender, bool generateUuid = false);
	static void RegisterOperation(const std::string& name, const std::optional<Username>& sender, const std::optional<DbId>& relatedItem, OperationOwnership ownership = OperationOwnership::undefined);
	static void PrepareCmdSend(CommandV2& cmd);
	static void AddOperationFields(DataStructure& dataStruct);
	static void ExecuteSyncRequest(Int64 sinceTimestamp, GenericConnection& destination);
};