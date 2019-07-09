#pragma once
#include "TypeDefs.hpp"
#include <any>
#include <optional>
#include "SQLValueType.hpp"
#include "SQLValue.hpp"
#include "MessagePack.hpp"
#include "CryptographyTypes.hpp"
#include "CustomDataType.h"
class NetworkPayload;
class JsonHelper;

enum class DataFieldType
{
	undefined,
	integer,
	boolean,
	text,
	byte_blob
};

struct DataField
{
	std::string name;
	DataFieldType basicType = DataFieldType::undefined;
	std::optional<std::string> customType;
	bool isRequired = true;
	bool isDataUnique = false;

	bool usedInServerDb = false;
	bool usedInClientDb = false;
	bool usedInCommad = false;

	std::optional<std::any> value;

	template <typename T>
	T GetValue()
	{
		if (customType.has_value())
			return CustomDataType::ConvertToCustomType<T>(value.value());
		else
			return std::any_cast<T>(value.value());
	}

	template <typename T>
	T GetValueOr(const T& optional)
	{
		if (value.has_value())
			return GetValue<T>();
		else
			return optional;
	}

	bool HasValue() const
	{
		return value.has_value();
	}

	bool Validate() const;

private:
	size_t GetLength() const
	{
		if (basicType == DataFieldType::integer)
			return std::any_cast<Int64>(value.value());

		else if (basicType == DataFieldType::byte_blob)
			return std::any_cast<ByteContainer>(value.value()).size();

		else if (basicType == DataFieldType::text)
			return std::any_cast<std::string>(value.value()).size();

		else
			return -1;
	}
};

struct DataStructure
{
	std::string name;
	std::vector<DataField> fields;
	std::optional<DbId> dbId;
	bool isValid = true;

	DataField& GetField(const std::string& fieldName)
	{
		auto& res = std::find_if(fields.begin(), fields.end(), [&fieldName](DataField& field)
		{
			return (field.name == fieldName);
		});

		if (res == fields.end())
			throw std::exception("The requested field do not exist");

		return *res;
	}

	MessagePackSerializer GetMessagePackFormat();
};

using DataStructureDef = DataStructure;

struct CustomFieldType
{
	std::string name;
	DataFieldType underlyingType;
	int minLength = -1;
	int maxLength = -1;
};

class ApoapseData
{
	std::vector<CustomFieldType> m_customTypes;
	std::vector<DataStructureDef> m_registeredDataStructures;

public:
	ApoapseData(const std::string& dataSchemeJson);
	const std::vector<DataStructureDef>& GetRegisteredStructures() const;
	const DataStructureDef& GetStructure(const std::string& name) const;	// Return a data structure object with definitions but no data 
	const CustomFieldType& GetCustomTypeInfo(const std::string& name) const;

	DataStructure ParseFromNetwork(const std::string& relatedDataStructure, std::shared_ptr<NetworkPayload> payload);

	void SaveToDatabase(const DataStructure& data) const;
	static bool IsStoredOnTheDatabase(const DataStructure& dataStructure);	// Check if the data structure provided has field stored on the database of the current platform (server or client)
	static bool IsStoredOnTheDatabase(const DataField& field);
	static SqlValueType ConvertFieldTypeToSqlType(const DataField& field);

	template <typename T>
	DataStructure ReadItemFromDatabase(const std::string& name, const std::string& searchBy, const T& searchValue)
	{
		return ReadItemFromDatabaseInternal(name, searchBy, SQLValue(searchValue, SQLValue::GenerateType<T>()));
	}

	std::vector<DataStructure> ReadListFromDatabase(const std::string& name, const std::string& seachFieldName) const
	{
		return std::vector<DataStructure>();
	}

private:
	void ReadDataStructures(const JsonHelper& json);
	void ReadCustomTypes(const JsonHelper& json);

	DataStructureDef& GetStructureDefinition(const std::string& name);
	DataFieldType GetTypeByTypeName(const std::string& typeStr) const;
	DataStructure ReadItemFromDatabaseInternal(const std::string& name, const std::string& seachFieldName, const SQLValue& searchValue);
};