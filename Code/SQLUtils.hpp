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