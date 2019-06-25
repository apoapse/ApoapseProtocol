#pragma once
#include "Json.hpp"
#include "SQLValueType.hpp"

class DatabaseIntegrityPatcher
{
	struct DbField
	{
		std::string name;
		SqlValueType type;
		std::string defaultValue;
		bool primary = false;
		bool unique = false;
		bool canBeNull = false;
#pragma region conparaisonOperators
		bool operator==(const DbField& other) const;

		bool operator!=(const DbField& other) const
		{
			return !(*this == other);
		}
#pragma endregion
	};

	struct DbTable 
	{
		std::string name;
		std::vector<DbField> fields;
#pragma region conparaisonOperators

		bool operator==(const DbTable& other)const;

		bool operator!=(const DbTable& other) const
		{
			return !(*this == other);
		}
#pragma endregion
	};

	std::vector<DbTable> m_SqlTables;

public:
	DatabaseIntegrityPatcher();
	bool CheckAndResolve();
	//virtual ~DatabaseIntegrityPatcher();
	
private:
	static SqlValueType ReadFieldType(const std::string& rawType);
	static std::string FieldTypeToStr(SqlValueType type);
	static DbTable ParseCreateTableQuery(const std::string& query);
	static std::string GenerateCreateTableQuery(const DbTable& table);
};