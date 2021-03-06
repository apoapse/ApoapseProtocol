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

#include "stdafx.h"
#include "Common.h"
#include "ApoapseData.h"
#include "Json.hpp"
#include "SQLQuery.h"
#include "NetworkPayload.h"
#include "CommandsManagerV2.h"
#include "ApoapseOperation.h"

ApoapseData::ApoapseData(const std::string& dataSchemeJson)
{
	JsonHelper json(dataSchemeJson);

	ReadCustomTypes(json);
	ReadDataStructures(json);
}

void ApoapseData::ReadDataStructures(const JsonHelper& json)
{
	for (const auto& dser : json.ReadFieldArray<JsonHelper>("data_structures"))
	{
		DataStructureDef dataStructure;
		dataStructure.name = dser.ReadFieldValue<std::string>("name").value();

		for (const auto& fieldDser : dser.ReadFieldArray<JsonHelper>("fields"))
		{
			DataField field;

			field.name = fieldDser.ReadFieldValue<std::string>("name").value();
			field.isRequired = fieldDser.ReadFieldValue<bool>("required").value_or(false);
			field.isDataUnique = fieldDser.ReadFieldValue<bool>("unique").value_or(false);
			field.usedInServerDb = fieldDser.ReadFieldValue<bool>("uses.server_storage").value_or(false);
			field.usedInClientDb = fieldDser.ReadFieldValue<bool>("uses.client_storage").value_or(false);
			field.usedInCommand = fieldDser.ReadFieldValue<bool>("uses.command").value_or(false);
			field.usedInClientUI = fieldDser.ReadFieldValue<bool>("uses.client_ui").value_or(false);
			field.readPermission = GetPermissionByName(fieldDser.ReadFieldValue<std::string>("propagate_to_clients").value_or("none"));

			{
				bool useCustomType = false;
				field.basicType = GetTypeByTypeName(fieldDser.ReadFieldValue<std::string>("type").value(), &useCustomType);

				if (useCustomType)
				{
					field.customType = fieldDser.ReadFieldValue<std::string>("type").value();
				}
			}

			if (field.basicType != DataFieldType::undefined)
				dataStructure.fields.push_back(field);
			else
				LOG << LogSeverity::error << "Type " << fieldDser.ReadFieldValue<std::string>("type").value() << " used on the structure " << dataStructure.name << " does not exist";
		}

		m_registeredDataStructures.push_back(dataStructure);
	}

	LOG << "Registered " << m_registeredDataStructures.size() << " data structures";
}

void ApoapseData::ReadCustomTypes(const JsonHelper& json)
{
	for (const auto& dser : json.ReadFieldArray<JsonHelper>("custom_types"))
	{
		CustomFieldType customType;
		
		bool useCustomType = false;

		customType.name = dser.ReadFieldValue<std::string>("name").value();
		customType.underlyingType = GetTypeByTypeName(dser.ReadFieldValue<std::string>("underlying_type").value(), &useCustomType);
		customType.minLength = dser.ReadFieldValue<int>("min_length").value_or(-1);
		customType.maxLength = dser.ReadFieldValue<int>("max_length").value_or(-1);
		customType.autoFill = dser.ReadFieldValue<bool>("auto_fill").value_or(false);

		m_customTypes.push_back(customType);
	}

	LOG << "Registered " << m_customTypes.size() << " custom data structure types";
}

void ApoapseData::FillAndValidate(MessagePackDeserializer& payloadData, DataStructure& data)
{
	for (auto& field : data.fields)
	{
		if (!field.usedInCommand || !data.isValid)
			continue;

		const bool valueExist = payloadData.Exist(field.name);

		if (field.isRequired && !valueExist)
		{
			LOG << "The field " << field.name << " is required but is not present in the net payload" << LogSeverity::error;
			data.isValid = false;
		}

		if (valueExist)
		{
			if (field.basicType == DataFieldType::boolean)
			{
				field.value = payloadData.GetValue<bool>(field.name);
			}
			else if (field.basicType == DataFieldType::byte_blob)
			{
				field.value = payloadData.GetValue<ByteContainer>(field.name);
			}
			else if (field.basicType == DataFieldType::integer)
			{
				field.value = payloadData.GetValue<Int64>(field.name);
			}
			else if (field.basicType == DataFieldType::text)
			{
				field.value = payloadData.GetValue<std::string>(field.name);
			}
			else if (field.basicType == DataFieldType::data_array)
			{
				std::vector<DataStructure> datArray;

				for (auto& dser : payloadData.GetArray<MessagePackDeserializer>(field.name))
				{
					DataStructure dat;

					{
						const auto& dataDef = GetStructureDefinition(field.customType.value());
						dat = dataDef;
					}

					FillAndValidate(dser, dat);
					datArray.push_back(dat);
				}

				field.SetValue(datArray);
			}

			data.isValid = field.Validate();
		}
	}
}

DataStructureDef& ApoapseData::GetStructureDefinition(const std::string& name)
{
	const auto res = std::find_if(m_registeredDataStructures.begin(), m_registeredDataStructures.end(), [&name](const DataStructureDef& dataStruct)
	{
		return (dataStruct.name == name);
	});

	if (res == m_registeredDataStructures.end())
		throw std::runtime_error("The requested data structure do not exist");

	return *res;
}

const std::vector<DataStructureDef>& ApoapseData::GetRegisteredStructures() const
{
	return m_registeredDataStructures;
}

const DataStructureDef& ApoapseData::GetStructure(const std::string& name) const
{
	const auto res = std::find_if(m_registeredDataStructures.begin(), m_registeredDataStructures.end(), [&name](const DataStructureDef& dataStruct)
	{
		return (dataStruct.name == name);
	});

	if (res == m_registeredDataStructures.end())
		throw std::runtime_error("The requested data structure do not exist");

	return *res;
}

const CustomFieldType* ApoapseData::GetCustomTypeInfo(const std::string& name) const
{
	const auto res = std::find_if(m_customTypes.begin(), m_customTypes.end(), [&name](const CustomFieldType& fieldType)
	{
		return (fieldType.name == name);
	});

	if (res == m_customTypes.end())
		return nullptr;

	return &*res;
}

bool ApoapseData::DataStructureExist(const std::string& name) const
{
	const auto res = std::find_if(m_registeredDataStructures.begin(), m_registeredDataStructures.end(), [&name](const DataStructureDef& dataStruct)
	{
		return (dataStruct.name == name);
	});

	return (res != m_registeredDataStructures.end());
}

DataStructure ApoapseData::FromNetwork(const CommandV2& cmd, std::shared_ptr<NetworkPayload> payload)
{
	DataStructure data;

	if (payload->headerInfo->payloadLength == 0)	// In case this is an empty command
		return data;
	
	std::vector<byte> networkPayload;

	{
		const auto range = payload->GetPayloadContent();
		networkPayload = std::vector<byte>(range.begin(), range.end());

		ASSERT(networkPayload.size() == payload->headerInfo->payloadLength);
	}

	MessagePackDeserializer payloadData(networkPayload);

	{
		const auto& dataDef = GetStructureDefinition(cmd.relatedDataStructure);
		data = dataDef;
	}

	if (cmd.operationRegister)
	{
		ApoapseOperation::AddOperationFields(data);
	}

	FillAndValidate(payloadData, data);

	return data;
}

DataStructure ApoapseData::FromJSON(const std::string& relatedDataStructure, const JsonHelper& json)
{
	DataStructure data;

	{
		const auto& dataDef = GetStructureDefinition(relatedDataStructure);
		data = dataDef;
	}

	for (auto& field : data.fields)
	{
		if (field.usedInCommand && json.ValueExist(field.name))
		{
			if (field.basicType == DataFieldType::boolean)
				field.value = json.ReadFieldValue<bool>(field.name).value();

			else if (field.basicType == DataFieldType::integer)
				field.value = json.ReadFieldValue<Int64>(field.name).value();

			else if (field.basicType == DataFieldType::text)
				field.value = json.ReadFieldValue<std::string>(field.name).value();

			else if (field.basicType == DataFieldType::byte_blob)
			{
				LOG_DEBUG << "The field " << field.name << " is supposed to be a BLOB but has been set as TEXT. Use CommandsManagerV2::OnSendCommandPre to replace it before sending.";
				field.value = json.ReadFieldValue<std::string>(field.name).value();
			}
		}
	}

	return data;
}

bool ApoapseData::AllowDatabaseStorage(const DataStructure& dataStructure)
{
	return std::count_if(dataStructure.fields.begin(), dataStructure.fields.end(), [](const DataField& field)
	{
		return AllowDatabaseStorage(field);
	});
}

bool ApoapseData::AllowDatabaseStorage(const DataField& field)
{
	return ((global->isClient && field.usedInClientDb) || (global->isServer && field.usedInServerDb));
}

DataFieldType ApoapseData::GetTypeByTypeName(const std::string& typeStr, bool* isCustomType) const
{
	*isCustomType = false;

	if (typeStr == "integer")
		return DataFieldType::integer;

	else if (typeStr == "bool")
		return DataFieldType::boolean;

	else if (typeStr == "text")
		return DataFieldType::text;

	else if (typeStr == "blob")
		return DataFieldType::byte_blob;

	else
	{
		*isCustomType = true;

		if (DataStructureExist(typeStr))
			return DataFieldType::data_array;
		else
			return GetCustomTypeInfo(typeStr)->underlyingType;
	}
}

ReadPermission ApoapseData::GetPermissionByName(const std::string& name)
{
	if (name == "all")
		return ReadPermission::all;

	else if (name == "author")
		return ReadPermission::author;

	else if (name == "usergroup")
		return ReadPermission::usergroup;

	else
		return ReadPermission::none;
}

DataStructure ApoapseData::ReadFromDbResult(DataStructureDef& dataStructDef, const SQLRow& row)
{
	DataStructure data = dataStructDef;
	int dbValueId = 0;

	data.SetDbId(row[0].GetInt64());
	dbValueId++;	// We set it to 1 to skip the id db field which is added by default

	for (auto& field : data.fields)
	{
		if (AllowDatabaseStorage(field))
		{
			if (row[dbValueId].GetType() != SqlValueType::UNSUPPORTED)
			{
				if (field.basicType == DataFieldType::boolean)
					field.value = row[dbValueId].GetBoolean();

				else if (field.basicType == DataFieldType::byte_blob)
					field.value = row[dbValueId].GetByteArray();

				else if (field.basicType == DataFieldType::integer)
					field.value = row[dbValueId].GetInt64();

				else if (field.basicType == DataFieldType::text)
					field.value = row[dbValueId].GetText();
			}

			dbValueId++;
		}
	}

	return data;
}

DataStructure ApoapseData::ReadFromDbResultCustomFields(DataStructureDef& dataStructDef, const SQLRow& row, const std::vector<std::string>& fieldsToRead)
{
	DataStructure data = dataStructDef;
	int dbValueId = 0;

	data.SetDbId(row[0].GetInt64());
	dbValueId++;	// We set it to 1 to skip the id db field which is added by default

	for (const std::string& fieldName : fieldsToRead)
	{
		auto& field = data.GetField(fieldName);

		if (field.basicType == DataFieldType::boolean)
			field.value = row[dbValueId].GetBoolean();

		else if (field.basicType == DataFieldType::byte_blob)
			field.value = row[dbValueId].GetByteArray();

		else if (field.basicType == DataFieldType::integer)
			field.value = row[dbValueId].GetInt64();

		else if (field.basicType == DataFieldType::text)
			field.value = row[dbValueId].GetText();

		dbValueId++;
	}
	
	return data;
}

DataStructure ApoapseData::ReadItemFromDbInternal(const std::string& name, const std::string& searchBy, const SQLValue& searchValue)
{
	auto& structureDef = GetStructureDefinition(name);

	SQLQuery query(*global->database);
	if (searchBy.empty())
		query << SELECT << ALL << FROM << structureDef.GetDBTableName().c_str();
	else
		query << SELECT << ALL << FROM << structureDef.GetDBTableName().c_str() << WHERE << searchBy.c_str() << EQUALS << searchValue;

	auto res = query.Exec();

	return ReadFromDbResult(structureDef, res[0]);
}

DataStructure ApoapseData::ReadItemFromDbInternalCustomFields(const std::string& name, const std::string& searchBy, const SQLValue& searchValue, const std::vector<std::string>& fieldsToRead)
{
	auto& structureDef = GetStructureDefinition(name);
	std::string fields;

	{
		std::vector<std::string> fieldsArr;
		fieldsArr.push_back("id");	// Make sure the id field is first
		std::copy(fieldsToRead.begin(), fieldsToRead.end(), std::back_inserter(fieldsArr));
		fields = StringExtensions::join(fieldsArr, ",", true);
	}
	
	SQLQuery query(*global->database);
	if (searchBy.empty())
		query << SELECT << fields.c_str() << FROM << structureDef.GetDBTableName().c_str();
	else
		query << SELECT << fields.c_str() << FROM << structureDef.GetDBTableName().c_str() << WHERE << searchBy.c_str() << EQUALS << searchValue;

	auto res = query.Exec();

	return ReadFromDbResultCustomFields(structureDef, res[0], fieldsToRead);
}

std::vector<DataStructure> ApoapseData::ReadListFromDbInternal(const std::string& name, const std::string& searchBy, const SQLValue& searchValue, const std::string& orderBy, ResultOrder order, Int64 limit)
{
	std::vector<DataStructure> ouput;
	auto& structureDef = GetStructureDefinition(name);

	SQLQuery query(*global->database);
	if (searchBy.empty())
		query << SELECT << ALL << FROM << structureDef.GetDBTableName().c_str();
	else
		query << SELECT << ALL << FROM << structureDef.GetDBTableName().c_str() << WHERE << searchBy.c_str() << EQUALS << searchValue;

	if (!orderBy.empty())
	{
		query << ORDER_BY << orderBy.c_str();
		
		if (order == ResultOrder::asc)
			query << ASC;
		else
			query << DESC;
	}

	if (limit != -1)
	{
		query << LIMIT << limit;
	}

	auto res = query.Exec();

	ouput.reserve(res.RowCount());
	for (const auto& row : res)
	{
		ouput.push_back(ReadFromDbResult(structureDef, row));
	}

	return ouput;
}
