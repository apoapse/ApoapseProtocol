// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once
#include "TypeDefs.hpp"
#include "MessagePack.hpp"
#include "CryptographyTypes.hpp"
#include "DataStructure.h"
class NetworkPayload;
class JsonHelper;
class SQLRow;
class CommandV2;

using DataStructureDef = DataStructure;

enum class ResultOrder
{
	desc,
	asc
};

class ApoapseData
{
	std::vector<CustomFieldType> m_customTypes;
	std::vector<DataStructureDef> m_registeredDataStructures;

public:
	ApoapseData(const std::string& dataSchemeJson);
	const std::vector<DataStructureDef>& GetRegisteredStructures() const;
	const DataStructureDef& GetStructure(const std::string& name) const;	// Return a data structure object with definitions but no data 
	const CustomFieldType* GetCustomTypeInfo(const std::string& name) const;
	bool DataStructureExist(const std::string& name) const;

	DataStructure FromNetwork(const CommandV2& cmd, std::shared_ptr<NetworkPayload> payload);
	DataStructure FromJSON(const std::string& relatedDataStructure, const JsonHelper& json);

	static bool AllowDatabaseStorage(const DataStructure& dataStructure);	// Check if the data structure provided has field stored on the database of the current platform (server or client)
	static bool AllowDatabaseStorage(const DataField& field);

	template <typename T>
	DataStructure ReadItemFromDBCustomFields(const std::string& name, const std::string& searchBy, const T& searchValue, const std::vector<std::string>& fieldsToRead)
	{
		return ReadItemFromDbInternalCustomFields(name, searchBy, GenerateSQLValue(searchValue), fieldsToRead);
	}

	template <typename T>
	DataStructure ReadItemFromDatabase(const std::string& name, const std::string& searchBy, const T& searchValue)
	{
		return ReadItemFromDbInternal(name, searchBy, GenerateSQLValue(searchValue));
	}

	template <typename T>
	std::vector<DataStructure> ReadListFromDatabase(const std::string& name, const std::string& searchBy, const T& searchValue, const std::string& orderBy = "", ResultOrder order = ResultOrder::desc, Int64 limit = -1)
	{
		return ReadListFromDbInternal(name, searchBy, GenerateSQLValue(searchValue), orderBy, order, limit);
	}

	DataStructure ReadFromDbResult(DataStructureDef& dataStructDef, const SQLRow& row);
	DataStructure ReadFromDbResultCustomFields(DataStructureDef& dataStructDef, const SQLRow& row, const std::vector<std::string>& fieldsToRead);

private:
	template <typename T>
	SQLValue GenerateSQLValue(const T& val)
	{
		SQLValue sqlVal;

		if constexpr (std::is_base_of<ICustomDataType, T>::value)
		{
			sqlVal = SQLValue(val.GetBytes(), SQLValue::GenerateType<ByteContainer>());
		}
		else
			sqlVal = SQLValue(val, SQLValue::GenerateType<T>());

		return sqlVal;
	}

	void ReadDataStructures(const JsonHelper& json);
	void ReadCustomTypes(const JsonHelper& json);

	void FillAndValidate(MessagePackDeserializer& payloadData, DataStructure& data);

	DataStructureDef& GetStructureDefinition(const std::string& name);
	DataFieldType GetTypeByTypeName(const std::string& typeStr, bool* isCustomType) const;
	static ReadPermission GetPermissionByName(const std::string& name);

	DataStructure ReadItemFromDbInternal(const std::string& name, const std::string& searchBy, const SQLValue& searchValue);
	DataStructure ReadItemFromDbInternalCustomFields(const std::string& name, const std::string& searchBy, const SQLValue& searchValue, const std::vector<std::string>& fieldsToRead);
	std::vector<DataStructure> ReadListFromDbInternal(const std::string& name, const std::string& searchBy, const SQLValue& searchValue, const std::string& orderBy, ResultOrder order, Int64 limit);
};