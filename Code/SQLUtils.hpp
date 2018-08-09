#pragma once
#include "SQLQuery.h"

namespace SQLUtils
{
	inline Int64 CountRows(const std::string& table)
	{
		SQLQuery query(*global->database);
		query << "SELECT Count(*) FROM " << table.c_str();
		auto res = query.Exec();

		return res[0][0].GetInt64();
	}
}