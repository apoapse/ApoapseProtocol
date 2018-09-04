#include "stdafx.h"
#include "Operation.h"
#include "Common.h"
#include "SQLQuery.h"
#include "DateTimeUtils.h"
#include "SQLUtils.hpp"

Operation::Operation(OperationType type, const Username& relatedUser, std::optional<DbId> itemDbId, OperationOwnership ownership /*= OperationOwnership::all*/)
	: type(type)
	, relatedUser(relatedUser)
	, ownership(ownership)
{
	if (itemDbId.has_value())
		this->itemDbId = itemDbId.value();
}

void Operation::Save()
{
	if (dbId != -1)
		throw std::exception("This operation is already saved");

	dbId = SQLUtils::CountRows("operations");
	time = DateTimeUtils::UnixTimestampNow();

	SQLQuery query(*global->database);
	query << INSERT_INTO << "operations" << " (id, type, time, item_dbid, ownership, related_user)" << VALUES <<
		"(" << dbId << "," << static_cast<Int32>(type) << "," << time << "," << itemDbId << "," << static_cast<Int32>(ownership) << "," << relatedUser.GetRaw() << ")";
	
	query.Exec();

	LOG_DEBUG << "Saved operation " << (int)type << " for user " << relatedUser.ToStr();
}

Int64 Operation::GetMostRecentOperationTime(const Username& relatedUser)
{
	SQLQuery query(*global->database);// #TODO #USERGROUP
	query << SELECT << "time" << FROM << "operations" << WHERE << "(" << "ownership" << EQUALS << static_cast<Int32>(OperationOwnership::all) << ")"
		<< OR << "(" << "related_user" << EQUALS << relatedUser.GetRaw() << ")"
		<< ORDER_BY << "time" << DESC << LIMIT << 1;
	
	auto res = query.Exec();

	if (res.RowCount() == 0)
		return 0;
	else
		return res[0][0].GetInt64();
}
