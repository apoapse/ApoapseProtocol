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