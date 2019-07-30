#include "stdafx.h"
#include "Common.h"
#include "ApoapseOperation.h"
#include "SQLQuery.h"
#include "CommandV2.h"
#include "DateTimeUtils.h"
#include "Uuid.h"
#include "GenericConnection.h"

void ApoapseOperation::RegisterOperation(CommandV2& cmd, const std::optional<Username>& sender)
{
	const std::string opName = cmd.name;

	auto opData = global->apoapseData->GetStructure("operation");
	opData.GetField("uuid").SetValue(cmd.GetData().GetField("operation_uuid").GetValue<Uuid>());
	opData.GetField("name").SetValue(opName);
	opData.GetField("time").SetValue(DateTimeUtils::UnixTimestampNow());

	if (global->isServer)
	{
		opData.GetField("ownership").SetValue((Int64)cmd.operationOwnership);

		if (sender.has_value() && cmd.operationOwnership == OperationOwnership::self || cmd.operationOwnership == OperationOwnership::usergroup)
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
	auto operationStruct = global->apoapseData->GetStructure("operation");

	UInt64 syncedItems = 0;
	for (const auto& sqlRow : res)
	{
		auto opData = global->apoapseData->ReadFromDbResult(operationStruct, sqlRow);
		const std::string cmdName = opData.GetField("name").GetValue<std::string>();
		const std::string dataStructName = global->cmdManager->GetCmdDefByFullName(cmdName).relatedDataStructure;

		auto itemData = global->apoapseData->ReadItemFromDatabase(dataStructName, "id", opData.GetField("item").GetValue<Int64>());

		AddOperationFields(itemData);
		itemData.GetField("operation_uuid").SetValue(opData.GetField("uuid").GetValue<Uuid>());

		auto cmd = global->cmdManager->CreateCommand(cmdName, itemData);
		cmd.Send(destination);

		syncedItems++;
	}

	LOG << "User requested sync since " << sinceTimestamp << ". " << syncedItems << " synchronized to " << destination.GetEndpointStr();
}
