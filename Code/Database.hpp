#pragma once
#include "TypeDefs.hpp"

class SQLValue;
class SQLPackagedResult;

struct IDatabase
{
	virtual ~IDatabase() = default;

	virtual bool Initialize() = 0;
	virtual bool Open(const char** params, size_t paramsCount) = 0;
	virtual void Close() = 0;

	virtual SQLPackagedResult ExecQuery(const char* preparedQuery, const SQLValue** values, size_t valuesCount) = 0;
	virtual const char* GetLastError() = 0;
};