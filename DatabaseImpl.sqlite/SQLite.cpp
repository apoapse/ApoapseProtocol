#include "SQLite.h"
#include "SQLValue.hpp"
#include "SQLPackagedResult.hpp"

SQLite::~SQLite()
{

}

bool SQLite::Open(const char** params, size_t paramsCount)
{
	if (paramsCount < 1)
		throw std::exception();

	if (sqlite3_config(SQLITE_CONFIG_SERIALIZED) != SQLITE_OK)
		return false;

	return (sqlite3_open(params[0], &m_database) == SQLITE_OK);
}

void SQLite::Close()
{
	sqlite3_close_v2(m_database);
}

SQLPackagedResult SQLite::ExecQuery(const char* preparedQuery, const SQLValue** values, size_t valuesCount)
{
	sqlite3_stmt* preparedStm;
	sqlite3_prepare_v2(m_database, preparedQuery, -1, &preparedStm, nullptr);

	// Bind values
	if (valuesCount > 0)
	{
		for (size_t i = 0; i < valuesCount; i++)
		{
			const SQLValue* value = values[i];

			switch (value->GetType())
			{
			case ValueType::INT:
				sqlite3_bind_int(preparedStm, (int)i + 1, value->GetInt32());
				break;

			case ValueType::INT_64:
				sqlite3_bind_int64(preparedStm, (int)i + 1, value->GetInt64());
				break;

			case ValueType::TEXT:
				sqlite3_bind_text(preparedStm, (int)i + 1, value->GetText().c_str(), -1, SQLITE_TRANSIENT);
				break;

			case ValueType::BYTE_ARRAY:
				auto data = value->GetByteArray();
				sqlite3_bind_blob(preparedStm, (int)i + 1, data.data(), (int)data.size(), SQLITE_TRANSIENT);
				break;
			}
		}
	}

	// Exec query
	int returnCode;
	
	bool hasResults = false;
	SQLPackagedResult resultContainer(true);
	
	while ((returnCode = sqlite3_step(preparedStm)) == SQLITE_ROW)
	{
		// Process results
		if (hasResults != true)
			hasResults = true;

		int columnCount = sqlite3_data_count(preparedStm);

		if (columnCount > 0)
		{
			SQLRow currentRow(columnCount);

			for (int i = 0; i < columnCount; i++)
			{
				switch (sqlite3_column_type(preparedStm, i))
				{
				case SQLITE_INTEGER:
					currentRow.AddValue(SQLValue((Int64)sqlite3_column_int64(preparedStm, i), ValueType::INT_64));
					break;

				case SQLITE_TEXT:
					currentRow.AddValue(SQLValue(std::move(std::string((const char*)sqlite3_column_text(preparedStm, i))), ValueType::TEXT));
					break;

				case SQLITE_BLOB:
					auto rawdata = (byte*)sqlite3_column_blob(preparedStm, i);
					int size = sqlite3_column_bytes(preparedStm, i);

					currentRow.AddValue(SQLValue(std::move(std::vector<byte>(rawdata, rawdata + size)), ValueType::BYTE_ARRAY));
					break;
				}
			}

			resultContainer.AddRow(currentRow);
		}
	}

	sqlite3_finalize(preparedStm);

	if (returnCode == SQLITE_DONE || hasResults)
		return resultContainer;
	else
		return SQLPackagedResult(false);
}

const char* SQLite::GetLastError()
{
	return sqlite3_errmsg(m_database);
}
