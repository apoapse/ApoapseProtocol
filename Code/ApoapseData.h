#pragma once
#include "TypeDefs.hpp"
#include <any>
#include <optional>
#include "SQLValueType.hpp"

enum class DataFieldType
{
	undefined,
	integer,
	boolean,
	text,
	byte_blob,
	uuid,
	username,
	datetime
};

struct DataField
{
	std::string name;
	DataFieldType type = DataFieldType::undefined;
	bool isRequired = true;
	bool isDataUnique = false;

	bool usedInServerDb = false;
	bool usedInClientDb = false;
	bool usedInCommad = false;

	std::optional<std::any> value;

	template <typename T>
	T GetValue()
	{
		std::any_cast<T>(value.value());
	}

	template <typename T>
	T GetValueOr(T optional)
	{
		std::any_cast<T>(value.value_or(optional));
	}

	bool HasValue() const
	{
		return value.has_value();
	}
};

struct DataStructure
{
	std::string name;
	std::vector<DataField> fields;
};

class ApoapseData
{
	std::vector<DataStructure> m_registeredDataStructures;

public:
	ApoapseData(const std::string& dataSchemeJson);

	const std::vector<DataStructure>& GetRegisteredStructures() const;
	static bool IsStoredOnTheDatabase(const DataStructure& dataStructure);	// Check if the data structure provided has field stored on the database of the current platform (server or client)
	static SqlValueType ConvertFieldTypeToSqlType(const DataField& field);

private:
	static DataFieldType GetTypeByTypeName(const std::string& typeStr);
};