#pragma once
#include "Json.hpp"

class DatabaseIntegrityPatcher
{
	struct DbField
	{
		enum class Type
		{
			unknown,
			integer,
			text,
			byte_blob,
			boolean
		};

		std::string name;
		Type type;
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
	DatabaseIntegrityPatcher(const std::string& dbSchemeJson);
	bool CheckAndResolve();
	//virtual ~DatabaseIntegrityPatcher();
	
private:
	static DbField::Type ReadFieldType(const std::string& rawType);
	static std::string FieldTypeToStr(const DbField::Type& type);
	static DbTable ParseCreateTableQuery(const std::string& query);
	static std::string GenerateCreateTableQuery(const DbTable& table);
};