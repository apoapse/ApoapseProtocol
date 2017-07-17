#pragma once
#include <sstream>
#include <vector>
#include "Database.hpp"
#include "SQLValue.hpp"
#include "SQLPackagedResult.hpp"
#include <future>
#include <string>
#include <sstream>

#define SELECT		"SELECT "
#define ALL			" * "
#define FROM		" FROM "
#define WHERE		" WHERE "
#define EQUALS		" = "
#define OR			" OR "
#define AND			" AND "
#define INSERT_INTO	"INSERT INTO "
#define VALUES		" VALUES "
#define UPDATE		"UPDATE "
#define SET			" SET "
#define ORDER_BY	" ORDER BY "
#define DESC		" DESC "
#define ASC			" ASC "
#define LIMIT		" LIMIT "

#define STR_TO_QUERY_SYNTAX(_str)	_str.c_str()

class DBException : public std::exception
{
	const std::string m_errorMsg;

public:
	DBException(std::string errorMsg) : m_errorMsg(std::move(errorMsg))
	{
	}

	virtual const char* what() const override
	{
		return m_errorMsg.c_str();
	}
};

class SQLQuery
{
	Database& m_database;
	std::stringstream m_query;
	std::vector<SQLValue*> m_values;

public:
	SQLQuery(Database& database) : m_database(database)
	{
	}

	virtual ~SQLQuery()
	{
		for (auto* ptr : m_values)
			delete ptr;

		m_values.clear();
	}

	const SQLPackagedResult Exec();

	std::future<SQLPackagedResult> ExecAsync();

	std::string GetPreparedFullQuery() const
	{
		return m_query.str();
	}

	//************************************
	// Method:    SQLQuery::operator<< - For anything that is part of a SQL query syntax
	// // Parameter: const char* queryStatement
	//************************************
	SQLQuery& operator<<(const char* queryStatement)
	{
		m_query << queryStatement;

		return *this;
	}

	//************************************
	// Method:    SQLQuery::operator<< - Handle the input value variables
	// // Parameter: const T & value
	//************************************
	template <typename T>
	SQLQuery& operator<<(const T& value)
	{
		AddValue(new SQLValue(value, SQLValue::GenerateType<T>()));

		return *this;
	}

	template <typename T>
	static std::string FormatArray(const std::vector<T> values)
	{
		std::stringstream ss;
		const size_t nbElements = values.size();

		for (size_t i = 0; i < nbElements; i++)
		{
			ss << values[i];

			if (i < nbElements - 1)
				ss << '\n';
		}

		return ss.str();
	}

	static std::vector<std::string> TextToArray(const std::string& str);

private:
	void AddValue(SQLValue* sqlValue);	// #TODO Add safety checks (when ENABLE_SEC_ADVANCED_CHECKS is defined)
};