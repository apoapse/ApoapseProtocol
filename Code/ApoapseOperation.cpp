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
#include "ApoapseOperation.h"
#include "SQLQuery.h"
#include "CommandV2.h"
#include "DateTimeUtils.h"
#include "Uuid.h"
#include "GenericConnection.h"

void ApoapseOperation::RegisterOperation(CommandV2& cmd, const std::optional<Username>& sender, bool generateUuid /*= false*/)
{
	const std::string opName = cmd.name;
	std::optional<Uuid> operationUuid = (cmd.GetData().FieldExist("operation_uuid") ? cmd.GetData().GetField("operation_uuid").GetValue<Uuid>() : std::optional<Uuid>());

	if (generateUuid)
	{
		operationUuid = Uuid::Generate();
	}
	
	if (!operationUuid.has_value())
	{
		PrepareCmdSend(cmd);
		LOG_DEBUG << "The command " << cmd.name << " do not have an operation uuid. It has been added. Make sure RegisterOperation is called before sending.";
	}

	auto opData = global->apoapseData->GetStructure("operation");
	opData.GetField("uuid").SetValue(operationUuid.value());
	opData.GetField("name").SetValue(opName);
	opData.GetField("time").SetValue(DateTimeUtils::UnixTimestampNow());

	if (global->isServer)
	{
		opData.GetField("ownership").SetValue((Int64)cmd.operationOwnership);

		if (sender.has_value() && (cmd.operationOwnership == OperationOwnership::self || cmd.operationOwnership == OperationOwnership::usergroup))
		{
			opData.GetField("related_user").SetValue(sender.value());
		}
	}

	if (ApoapseData::AllowDatabaseStorage(cmd.GetData()))
	{
		opData.GetField("item").SetValue(cmd.GetData().GetDbId());
	}

	opData.SaveToDatabase();
	LOG << "Saved operation " << opName;
}

void ApoapseOperation::RegisterOperation(const std::string& name, const std::optional<Username>& sender, const std::optional<DbId>& relatedItem, OperationOwnership ownership)
{
	auto opData = global->apoapseData->GetStructure("operation");
	opData.GetField("uuid").SetValue(Uuid::Generate());
	opData.GetField("name").SetValue(name);
	opData.GetField("time").SetValue(DateTimeUtils::UnixTimestampNow());

	if (global->isServer)
	{
		opData.GetField("ownership").SetValue((Int64)ownership);
		
		if (sender.has_value() && (ownership == OperationOwnership::self || ownership == OperationOwnership::usergroup))
		{
			opData.GetField("related_user").SetValue(sender.value());
		}
	}

	if (relatedItem.has_value())
	{
		opData.GetField("item").SetValue(relatedItem.value());
	}

	opData.SaveToDatabase();
	LOG << "Saved operation " << name << " manually";
}

void ApoapseOperation::PrepareCmdSend(CommandV2& cmd)
{
	ASSERT(cmd.operationRegister);

	if (!cmd.GetData().FieldExist("operation_uuid"))
	{
		AddOperationFields(cmd.GetData());
		cmd.GetData().GetField("operation_uuid").SetValue(Uuid::Generate());
	}
}

void ApoapseOperation::AddOperationFields(DataStructure& dataStruct)
{
	DataField field;
	field.name = "operation_uuid";
	field.usedInCommand = true;
	field.isRequired = true;
	field.isDataUnique = true;
	field.customType = "uuid";
	field.basicType = DataFieldType::byte_blob;

	dataStruct.fields.push_back(field);
}

void ApoapseOperation::ExecuteSyncRequest(Int64 sinceTimestamp, GenericConnection& destination)
{
	ASSERT(global->isServer);

	SQLQuery query(*global->database);
	query << SELECT << ALL << FROM << "operations" << WHERE << "time" << GREATER_THAN << sinceTimestamp << AND << "(" << "ownership" << EQUALS << (Int64)OperationOwnership::all << OR << "related_user" << EQUALS << destination.GetConnectedUser().value().GetBytes() << ")";
	auto res = query.Exec();
	DataStructureDef operationStruct = global->apoapseData->GetStructure("operation");

	if (res.RowCount() > 0)
	{
		DataStructure dat = global->apoapseData->GetStructure("sync_info");
		dat.GetField("nb_items").SetValue((Int64)res.RowCount());

		global->cmdManager->CreateCommand("start_sync", dat).Send(destination);
	}

	UInt64 syncedItems = 0;
	for (const auto& sqlRow : res)
	{
		auto opData = global->apoapseData->ReadFromDbResult(operationStruct, sqlRow);
		const std::string cmdName = opData.GetField("name").GetValue<std::string>();
		const CommandV2Def cmdDef = global->cmdManager->GetCmdDefByFullName(cmdName);

		DataStructure itemData = global->apoapseData->ReadItemFromDatabase(cmdDef.operationDatastructure, "id", opData.GetField("item").GetValue<Int64>());

		//TODO filter

		AddOperationFields(itemData);
		itemData.GetField("operation_uuid").SetValue(opData.GetField("uuid").GetValue<Uuid>());

		auto cmd = global->cmdManager->CreateCommand(cmdName, itemData);
		cmd.Send(destination);

		syncedItems++;
	}

	LOG << "User requested sync since " << sinceTimestamp << ". " << syncedItems << " synchronized to " << destination.GetEndpointStr();
}
