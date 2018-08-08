#include "stdafx.h"
#include "DatabaseIntegrityPatcher.h"
#include "Common.h"
#include "Json.hpp"
#include "SQLQuery.h"
#include <regex>
#include "StringExtensions.h"

bool DatabaseIntegrityPatcher::DbTable::operator==(const DbTable& other) const
{
	if (other.name != name)
	{
		LOG << "The two tables have a different name" << LogSeverity::verbose;
		return false;
	}

	if (other.fields.size() != fields.size())
	{
		LOG << "The two tables have a different number of fields" << LogSeverity::verbose;
		return false;
	}

	for (size_t i = 0; i < fields.size(); i++)
	{
		if (other.fields[i] != fields[i])
			return false;
	}

	return true;
}

bool DatabaseIntegrityPatcher::DbField::operator==(const DbField& other) const
{
	if (other.name != name)
	{
		LOG << "The two fields have a different name" << LogSeverity::verbose;
		return false;
	}

	if (other.defaultValue != defaultValue)
	{
		LOG << "The two fields have a different defaultValue" << LogSeverity::verbose;
		return false;
	}

	if (other.primary != primary)
	{
		LOG << "The two fields are not primary keys" << LogSeverity::verbose;
		return false;
	}

	if (other.unique != unique)
	{
		LOG << "The two fields have not the unique flag" << LogSeverity::verbose;
		return false;
	}

	if (other.canBeNull != canBeNull)
	{
		LOG << "The two fields have not the same canBeNull flag" << LogSeverity::verbose;
		return false;
	}

	return true;
}


DatabaseIntegrityPatcher::DatabaseIntegrityPatcher(const std::string& dbSchemeJson)
{
	JsonHelper json(dbSchemeJson);

	const auto tablesArr = json.ReadFieldArray<JsonHelper>("tables");
	for (const auto& tableJson : tablesArr)
	{
		DbTable table;
		table.name = tableJson.ReadFieldValue<std::string>("name").value();

		const auto fieldsArr = tableJson.ReadFieldArray<JsonHelper>("fields");
		for (const auto& fieldJson : fieldsArr)
		{
			DbField field;
			field.name = fieldJson.ReadFieldValue<std::string>("name").value();
			field.type = ReadFieldType(fieldJson.ReadFieldValue<std::string>("type").value());

			if (fieldJson.ReadFieldValue<std::string>("defaultValue").is_initialized())
				field.defaultValue = fieldJson.ReadFieldValue<std::string>("defaultValue").value();

			if (fieldJson.ReadFieldValue<bool>("unique").is_initialized())
				field.unique = fieldJson.ReadFieldValue<bool>("unique").value();

			if (fieldJson.ReadFieldValue<bool>("canBeNull").is_initialized())
				field.canBeNull = fieldJson.ReadFieldValue<bool>("canBeNull").value();

			if (fieldJson.ReadFieldValue<bool>("primary").is_initialized() && (fieldJson.ReadFieldValue<bool>("primary").value() == true))
			{
				field.primary = true;
				field.canBeNull = false;
			}

			table.fields.push_back(field);
		}

		m_SqlTables.push_back(table);
	}
}

bool DatabaseIntegrityPatcher::CheckAndResolve()
{
	LOG << "Starting database integrity patcher";
	UInt32 tablesCreatedCount = 0;
	UInt32 tablesUpdatedCount = 0;

	for (const auto& table : m_SqlTables)
	{
		SQLQuery query(*global->database);
		query << "SELECT sql FROM sqlite_master WHERE name = '" << STR_TO_QUERY_SYNTAX(table.name) << "'";
		auto res = query.Exec();
		
		if (res.RowCount() > 0)
		{
			DbTable tableOnDb = ParseCreateTableQuery(res[0][0].GetText());

 			try
 			{
				if (tableOnDb != table)
				{
					// The table need to be updated
					const std::string tempTableName = "temp_" + tableOnDb.name;

					// Rename the actual table to a temporary name
					{
						SQLQuery query(*global->database);
						query << "ALTER TABLE '" << STR_TO_QUERY_SYNTAX(tableOnDb.name) << "' RENAME TO " << STR_TO_QUERY_SYNTAX(tempTableName);
						if (!query.Exec())
							return false;
					}

					// Create the new table
					{
						SQLQuery query(*global->database);
						query << GenerateCreateTableQuery(table).c_str();
						if (!query.Exec())
							return false;
					}

					// Populate the new table with old data
					{
						std::string fieldsList;	// #TODO Currently we can only add now fields

						for (size_t i = 0; i < tableOnDb.fields.size(); i++)
						{
							fieldsList += tableOnDb.fields[i].name;

							if (i < (tableOnDb.fields.size() - 1))
								fieldsList += ", ";
						}

						SQLQuery query(*global->database);
						query << "INSERT INTO '" << STR_TO_QUERY_SYNTAX(table.name) << "' (" << STR_TO_QUERY_SYNTAX(fieldsList) <<") SELECT " << STR_TO_QUERY_SYNTAX(fieldsList)<< " FROM " << STR_TO_QUERY_SYNTAX(tempTableName);
						if (!query.Exec())
							return false;
					}

					// Remove the old table
					{
						SQLQuery query(*global->database);
						query << "DROP TABLE " << STR_TO_QUERY_SYNTAX(tempTableName);
						if (!query.Exec())
							return false;
					}

					tablesUpdatedCount++;
				}
			}
			catch (const std::exception&)
			{
				return false;
			}
		}
		else
		{
			// The table need to be created
			LOG << "Creating table " << table.name << " has it does not exist on the database currently";

			SQLQuery query(*global->database);
			query << GenerateCreateTableQuery(table).c_str();
			if (!query.Exec())
				return false;

			tablesCreatedCount++;
		}
	}

	LOG << "Database integrity patcher ended. " << tablesCreatedCount << " new tables created and " << tablesUpdatedCount << " tables updated.";

	return true;
}

DatabaseIntegrityPatcher::DbField::Type DatabaseIntegrityPatcher::ReadFieldType(const std::string& rawType)
{
	const std::string strType = StringExtensions::ToLowercase(rawType);

	if (strType == "integer" || strType == "int")
		return DbField::Type::integer;

	else if (strType == "text")
		return DbField::Type::text;

	else if (strType == "blob")
		return DbField::Type::byte_blob;

	else if (strType == "bool")
		return DbField::Type::boolean;

	else
	{
		LOG << LogSeverity::error << "Database scheme: Unsupported field type " << rawType;
		return DbField::Type::unknown;
	}
}

std::string DatabaseIntegrityPatcher::FieldTypeToStr(const DbField::Type& type)
{
	switch (type)
	{
		case DbField::Type::integer:
			return "INTEGER";

		case DbField::Type::text:
			return "TEXT";

		case DbField::Type::byte_blob:
			return "BLOB";

		case DbField::Type::boolean:
			return "INTEGER";

		default:
			return "UNKNOWN";
	}
}

DatabaseIntegrityPatcher::DbTable DatabaseIntegrityPatcher::ParseCreateTableQuery(const std::string& query)
{
	DbTable table;

	std::vector<std::string> individualLines;
	StringExtensions::split(query, individualLines, "\n");
	if (individualLines.size() < 2)
		return table;

	// Table info
	{
		std::regex expr(R"(CREATE TABLE '(\S*)')");
		std::smatch match;

		if (std::regex_search(query, match, expr))
		{
			table.name = match[1];
		}
	}

	// Fields info
	std::for_each(individualLines.begin() + 1, individualLines.end() - 1, [&](std::string const& line)
	{
		const auto lineEndItr = (line[line.length() - 1] == ',') ? line.end() - 1 : line.end();
		std::string sqlFlags;

		std::regex expr(R"('(\S*)'\s*(\S*)\s*(.*))");
		std::smatch match;

		if (std::regex_search(line.begin(), lineEndItr, match, expr))
		{
			DbField field;
			field.name = match[1];
			field.type = ReadFieldType(match[2]);
			sqlFlags = match[3];

			if (StringExtensions::contains(line, "UNIQUE"))
				field.unique = true;

			if (StringExtensions::contains(line, "NOT NULL"))
				field.canBeNull = false;
			else
				field.canBeNull = true;

			if (StringExtensions::contains(line, "PRIMARY KEY"))
			{
				field.primary = true;
				field.canBeNull = false;
			}

			table.fields.push_back(field);
		}
	});

	return table;
}

std::string DatabaseIntegrityPatcher::GenerateCreateTableQuery(const DbTable& table)
{
	std::string strQuery;

	strQuery += "CREATE TABLE '" + table.name + "' (\n";

	for (size_t i = 0; i < table.fields.size(); i++)
	{
		const DbField& field = table.fields[i];

		strQuery += "'" + field.name + "' " + FieldTypeToStr(field.type);

		if (!field.canBeNull)
			strQuery += " NOT NULL";

		if (field.unique)
			strQuery += " UNIQUE";

		if (field.primary)
			strQuery += " PRIMARY KEY";

		if (i < (table.fields.size() - 1))
			strQuery += ",";

		strQuery += "\n";
	}

	strQuery += ")";

	return strQuery;
}