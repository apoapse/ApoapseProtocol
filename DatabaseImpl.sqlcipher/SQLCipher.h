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

	bool Open(const char** params, size_t paramsCount) override;
	void Close() override;

	SQLPackagedResult ExecQuery(const char* preparedQuery, const SQLValue** values, size_t valuesCount) override;
	const char* GetLastError() override;
};

extern "C" BOOST_SYMBOL_EXPORT SQLCipher apoapse_internal_dll;
SQLCipher apoapse_internal_dll;