#include "stdafx.h"
#include "SQLQuery.h"
#include "Common.h"
#include "StringExtensions.hpp"

SQLQuery::~SQLQuery()
{
	for (auto* ptr : m_values)
		delete ptr;

	m_values.clear();
}

const SQLPackagedResult SQLQuery::Exec()
{
	const std::string preparedQuery = GetPreparedFullQuery();
	const SQLValue** valuesArray = (!m_values.empty()) ? (const SQLValue**)m_values.data() : nullptr;

	const auto result = m_database.ExecQuery(preparedQuery.c_str(), valuesArray, m_values.size());

	if (!result)
	{
		const auto strError = std::string(m_database.GetLastError());
		throw DBException("SQL error on query [" + preparedQuery + "] - " + strError);
	}

	return result;
}

std::future<SQLPackagedResult> SQLQuery::ExecAsync()
{
	return global->threadPool->PushTask([this]
	{
		return Exec();
	});
}

std::vector<std::string> SQLQuery::TextToArray(const std::string& str)
{
	std::vector<std::string> output;

	StringExtensions::split(str, output, "\n");

	return output;
}

void SQLQuery::AddValue(SQLValue* sqlValue) // #TODO Add safety checks (when ENABLE_SEC_ADVANCED_CHECKS is defined)
{
	m_values.push_back(sqlValue);
	m_query << "?";	// Insert the value placeholder
}
