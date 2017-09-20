#pragma once
#include "TypeDefs.hpp"
#include "Database.hpp"
#include <boost/config.hpp>
#include <SQLite3.h>

class SQLite : public IDatabase
{
	sqlite3* m_database;

public:
	~SQLite();

	bool Open(const char** params, size_t paramsCount) override;
	void Close() override;

	SQLPackagedResult ExecQuery(const char* preparedQuery, const SQLValue** values, size_t valuesCount) override;
	const char* GetLastError() override;
};

extern "C" BOOST_SYMBOL_EXPORT SQLite apoapse_internal_dll;
SQLite apoapse_internal_dll;