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
#include <stdexcept>
#include "Database.hpp"
#include <boost/config.hpp>

#define SQLITE_HAS_CODEC
#include <SQLite3.h>

class SQLCipher : public IDatabase
{
	sqlite3* m_database;

public:
	~SQLCipher();

	bool Initialize() override;
	bool Open(const char** params, size_t paramsCount) override;
	void Close() override;

	SQLPackagedResult ExecQuery(const char* preparedQuery, const SQLValue** values, size_t valuesCount) override;
	const char* GetLastError() override;
};

extern "C" BOOST_SYMBOL_EXPORT SQLCipher apoapse_internal_dll;
SQLCipher apoapse_internal_dll;