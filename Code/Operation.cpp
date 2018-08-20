#include "stdafx.h"
#include "Operation.h"
#include "Common.h"
#include "SQLQuery.h"
#include "DateTimeUtils.h"
#include "SQLUtils.hpp"

Operation::Operation(OperationType type, const Username& user, std::optional<DbId> itemDbId) : type(type), user(user)
{
	if (itemDbId.has_value())
		this->itemDbId = itemDbId.value();
}

// void Operation::SaveAndSend(INetworkSender& destination)
// {
// 	Save();
// 
// 	//TODO
// }

void Operation::Save()
{
	if (dbId != -1)
		throw std::exception("This operation is already saved");

	dbId = SQLUtils::CountRows("operations");
	time = DateTimeUtils::UnixTimestampNow();

	SQLQuery query(*global->database);
	query << INSERT_INTO << "operations" << " (id, type, time, item_dbid)" << VALUES << "(" << dbId << "," << static_cast<Int32>(type) << "," << time << "," << itemDbId << ")";
	
	query.Exec();
}

Int64 Operation::GetMostRecentOperationTime()
{
	SQLQuery query(*global->database);
	query << SELECT << "time" << FROM << "operations" << ORDER_BY << "time" << DESC << LIMIT << 1;
	auto res = query.Exec();

	if (res.RowCount() == 0)
		return 0;
	else
		return res[0][0].GetInt64();
}
