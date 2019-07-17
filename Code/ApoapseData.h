#pragma once
#include "TypeDefs.hpp"
#include "MessagePack.hpp"
#include "CryptographyTypes.hpp"
#include "DataStructure.h"
class NetworkPayload;
class JsonHelper;

using DataStructureDef = DataStructure;

class ApoapseData
{
	std::vector<CustomFieldType> m_customTypes;
	std::vector<DataStructureDef> m_registeredDataStructures;

public:
	ApoapseData(const std::string& dataSchemeJson);
	const std::vector<DataStructureDef>& GetRegisteredStructures() const;
	const DataStructureDef& GetStructure(const std::string& name) const;	// Return a data structure object with definitions but no data 
	const CustomFieldType& GetCustomTypeInfo(const std::string& name) const;

	DataStructure FromNetwork(const std::string& relatedDataStructure, std::shared_ptr<NetworkPayload> payload);
	DataStructure FromJSON(const std::string& relatedDataStructure, const JsonHelper& json);

	static bool AllowDatabaseStorage(const DataStructure& dataStructure);	// Check if the data structure provided has field stored on the database of the current platform (server or client)
	static bool AllowDatabaseStorage(const DataField& field);

	template <typename T>
	DataStructure ReadItemFromDatabase(const std::string& name, const std::string& searchBy, const T& searchValue)
	{
		return ReadItemFromDatabaseInternal(name, searchBy, SQLValue(searchValue, SQLValue::GenerateType<T>()));
	}

	std::vector<DataStructure> ReadListFromDatabase(const std::string& name, const std::string& seachFieldName) const
	{
		return std::vector<DataStructure>();//TODODODODODODDODODOODOD
	}

private:
	void ReadDataStructures(const JsonHelper& json);
	void ReadCustomTypes(const JsonHelper& json);

	DataStructureDef& GetStructureDefinition(const std::string& name);
	DataFieldType GetTypeByTypeName(const std::string& typeStr, bool* isCustomType) const;

	DataStructure ReadItemFromDatabaseInternal(const std::string& name, const std::string& seachFieldName, const SQLValue& searchValue);
};