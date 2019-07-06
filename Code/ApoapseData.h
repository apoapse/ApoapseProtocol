#pragma once
#include "TypeDefs.hpp"
#include <any>
#include <optional>
#include "SQLValueType.hpp"
#include "SQLValue.hpp"
#include "MessagePack.hpp"
class NetworkPayload;

enum class DataFieldType	//REPLACE WITH BASIC TYPES AND USE A JSON THING TO SET TYPE CHECK FOR CUSTOM WITH A FUNCTION FOR EACH TYPE OF CHECKS: OnCheckType(ByteContainer& data, string& typeName)
{
	undefined,
	integer,
	boolean,
	text,
	byte_blob,
	uuid,
	username,
	datetime,
	timestamp
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
		return std::any_cast<T>(value.value());
	}

	template <typename T>
	T GetValueOr(T optional)
	{
		return std::any_cast<T>(value.value_or(optional));
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

class ApoapseData
{
	std::vector<DataStructureDef> m_registeredDataStructures;

public:
	ApoapseData(const std::string& dataSchemeJson);
	const std::vector<DataStructureDef>& GetRegisteredStructures() const;
	const DataStructureDef& GetStructure(const std::string& name) const;	// Return a data structure object with definitions but no data 

	DataStructure ParseFromNetwork(const std::string& relatedDataStructure, std::shared_ptr<NetworkPayload> payload);

	static bool IsStoredOnTheDatabase(const DataStructure& dataStructure);	// Check if the data structure provided has field stored on the database of the current platform (server or client)
	static bool IsStoredOnTheDatabase(const DataField& field);
	static SqlValueType ConvertFieldTypeToSqlType(const DataField& field);

	template <typename T>
	DataStructure ReadItemFromDatabase(const std::string& name, const std::string& seachFieldName, const T& searchValue)
	{
		return ReadItemFromDatabaseInternal(name, seachFieldName, SQLValue(searchValue, SQLValue::GenerateType<T>()));
	}

	std::vector<DataStructure> ReadListFromDatabase(const std::string& name, const std::string& seachFieldName) const
	{
		return std::vector<DataStructure>();
	}

private:
	DataStructureDef& GetStructureDefinition(const std::string& name);
	static DataFieldType GetTypeByTypeName(const std::string& typeStr);
	DataStructure ReadItemFromDatabaseInternal(const std::string& name, const std::string& seachFieldName, const SQLValue& searchValue);
};