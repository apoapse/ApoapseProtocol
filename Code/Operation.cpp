#include "stdafx.h"
#include "Operation.h"
#include "Common.h"
#include "SQLQuery.h"
#include "DateTimeUtils.h"

Operation::Operation(OperationType type, OperationDirection direction, const Username& user, std::optional<Uuid> itemUuid)
	: type(type)
	, direction(direction)
	, user(user)
	, itemUuid(itemUuid)
{

}

// void Operation::SaveAndSend(INetworkSender& destination)
// {
// 	Save();
// 
// 	//TODO
// }

void Operation::Save()
{
	if (m_dbId != -1)
		throw std::exception("This operation is already saved");

	m_dbId = GetPreviousDbId();
	time = DateTimeUtils::UnixTimestampNow();

	SQLQuery query(*global->database);
	if (itemUuid.has_value())
		query << INSERT_INTO << "operations" << " (id, type, direction, time, user, item_uuid)" << VALUES << "(" << m_dbId << "," << static_cast<Int32>(type) << "," << static_cast<Int32>(direction) << "," << time << "," << user.GetRaw() << "," << itemUuid.value().GetInRawFormat() << ")";
 	else
 		query << INSERT_INTO << "operations" << " (id, type, direction, time, user)" << VALUES << "(" << m_dbId << "," << static_cast<Int32>(type) << "," << static_cast<Int32>(direction) << "," << time << "," << user.GetRaw() << ")";

	query.Exec();
}

DbId Operation::GetPreviousDbId()
{
	SQLQuery query(*global->database);
	query << SELECT << "id" << FROM << "operations" << ORDER_BY << "id" << DESC << LIMIT << "1";
	auto res = query.Exec();

	if (res && res.RowCount() == 1)
		return (res[0][0].GetInt64() + 1);
	else
		return 0;
}
