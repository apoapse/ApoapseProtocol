#include "stdafx.h"
#include "Common.h"
#include "ApoapseOperation.h"
#include "SQLQuery.h"
#include "CommandV2.h"
#include "DateTimeUtils.h"

void ApoapseOperation::SaveOperation(CommandV2& cmd, const std::optional<Username>& sender)
{
	const std::string opName = cmd.name;

	auto opData = global->apoapseData->GetStructure("operation");
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
