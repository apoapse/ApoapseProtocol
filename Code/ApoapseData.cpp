#include "stdafx.h"
#include "Common.h"
#include "ApoapseData.h"
#include "Json.hpp"
#include "SQLQuery.h"
#include "NetworkPayload.h"
#include "CommandsManagerV2.h"

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
			field.usedInCommad = fieldDser.ReadFieldValue<bool>("uses.command").value_or(false);

			bool useCustomType = false;
			field.basicType = GetTypeByTypeName(fieldDser.ReadFieldValue<std::string>("type").value(), &useCustomType);
			
			if (useCustomType)
			{
				field.customType = fieldDser.ReadFieldValue<std::string>("type").value();
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

		m_customTypes.push_back(customType);
	}

	LOG << "Registered " << m_customTypes.size() << " custom data structure types";
}

DataStructureDef& ApoapseData::GetStructureDefinition(const std::string& name)
{
	auto& res = std::find_if(m_registeredDataStructures.begin(), m_registeredDataStructures.end(), [&name](const DataStructureDef& dataStruct)
	{
		return (dataStruct.name == name);
	});

	if (res == m_registeredDataStructures.end())
		throw std::exception("The requested data structure do not exist");

	return *res;
}

const std::vector<DataStructureDef>& ApoapseData::GetRegisteredStructures() const
{
	return m_registeredDataStructures;
}

const DataStructureDef& ApoapseData::GetStructure(const std::string& name) const
{
	auto& res = std::find_if(m_registeredDataStructures.begin(), m_registeredDataStructures.end(), [&name](const DataStructureDef& dataStruct)
	{
		return (dataStruct.name == name);
	});

	if (res == m_registeredDataStructures.end())
		throw std::exception("The requested data structure do not exist");

	return *res;
}

const CustomFieldType& ApoapseData::GetCustomTypeInfo(const std::string& name) const
{
	auto& res = std::find_if(m_customTypes.begin(), m_customTypes.end(), [&name](const CustomFieldType& fieldType)
		{
			return (fieldType.name == name);
		});

	if (res == m_customTypes.end())
		throw std::exception("The requested custom filed type do not exist");

	return *res;
}

DataStructure ApoapseData::ParseFromNetwork(const std::string& relatedDataStructure, std::shared_ptr<NetworkPayload> payload)
{
	DataStructure data;
	std::vector<byte> networkPayload;

	{
		const auto range = payload->GetPayloadContent();
		networkPayload = std::vector<byte>(range.begin(), range.end());

		ASSERT(networkPayload.size() == payload->headerInfo->payloadLength);
	}

	MessagePackDeserializer payloadData(networkPayload);

	{
		const auto& dataDef = GetStructureDefinition(relatedDataStructure);
		data = dataDef;
	}

	for (auto& field : data.fields)
	{
		if (!field.usedInCommad)
			continue;

		if (field.isRequired && !payloadData.Exist(field.name))
		{
			LOG << "The field " << field.name << " is required but is not present in the net payload" << LogSeverity::error;
			data.isValid = false;
		}

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

		data.isValid = field.Validate();
	}

	return data;
}

void ApoapseData::SaveToDatabase(const DataStructure& data) const
{
	const int nbFieldToSave = std::count_if(data.fields.begin(), data.fields.end(), [](const DataField& field)
	{
		return (field.usedInClientDb == global->isClient || field.usedInServerDb && global->isServer);
	});

	if (nbFieldToSave == 0)
	{
		LOG << LogSeverity::error << "Trying to save data of type " << data.name << " but none of the fields are set to be saved on the database";
		return;
	}

	//TOODODODOD
}

bool ApoapseData::IsStoredOnTheDatabase(const DataStructure& dataStructure)
{
	return std::count_if(dataStructure.fields.begin(), dataStructure.fields.end(), [](const DataField& field)
	{
		return IsStoredOnTheDatabase(field);
	});
}

bool ApoapseData::IsStoredOnTheDatabase(const DataField& field)
{
	return (global->isClient && field.usedInClientDb || global->isServer && field.usedInServerDb);
}

SqlValueType ApoapseData::ConvertFieldTypeToSqlType(const DataField& field)
{
	switch (field.basicType)
	{
	case DataFieldType::integer:
		return SqlValueType::INT_64;

	case DataFieldType::boolean:
		return SqlValueType::INT;

	case DataFieldType::byte_blob:
		return SqlValueType::BYTE_ARRAY;

	case DataFieldType::text:
		return SqlValueType::TEXT;

	default:
		return SqlValueType::UNSUPPORTED;
	}
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
		return GetCustomTypeInfo(typeStr).underlyingType;
	}
}

DataStructure ApoapseData::ReadItemFromDatabaseInternal(const std::string& name, const std::string& searchBy, const SQLValue& searchValue)
{
	auto& structureDef = GetStructureDefinition(name);
	//auto& fieldInfo = structureDef.GetField(seachFieldName);

	SQLQuery query(*global->database);
	query << SELECT << ALL << FROM << std::string(name + "s").c_str() << WHERE << searchBy.c_str() << EQUALS << searchValue;
	auto res = query.Exec();

	DataStructure data = structureDef;

	data.dbId = res[0][0].GetInt64();

	int dbValueId = 1;	// We start at id 1 to skip the id db field which is added by default 
	for (auto& field : data.fields)
	{
		if (IsStoredOnTheDatabase(field))
		{
			if (field.basicType == DataFieldType::boolean)
				field.value = res[0][dbValueId].GetBoolean();

			if (field.basicType == DataFieldType::byte_blob)
				field.value = res[0][dbValueId].GetByteArray();

			if (field.basicType == DataFieldType::integer)
				field.value = res[0][dbValueId].GetInt64();

			if (field.basicType == DataFieldType::text)
				field.value = res[0][dbValueId].GetText();

			dbValueId++;
		}
	}

	return data;
}

MessagePackSerializer DataStructure::GetMessagePackFormat()
{
	MessagePackSerializer ser;

	for (auto& field : fields)
	{
		if (field.basicType == DataFieldType::boolean)
			ser.UnorderedAppend<bool>(field.name, field.GetValue<bool>());

		if (field.basicType == DataFieldType::byte_blob)
			ser.UnorderedAppend<ByteContainer>(field.name, field.GetValue<ByteContainer>());

		if (field.basicType == DataFieldType::integer)
			ser.UnorderedAppend<Int64>(field.name, field.GetValue<Int64>());

		if (field.basicType == DataFieldType::text)
			ser.UnorderedAppend<std::string>(field.name, field.GetValue<std::string>());
	}

	return ser;
}

inline bool DataField::Validate() const
{
	bool isValid = true;

	if (customType.has_value())
	{
		const auto typeDef = global->apoapseData->GetCustomTypeInfo(customType.value());

		// Validate ranges
		if (typeDef.minLength != -1 || typeDef.maxLength != -1)
		{
			const size_t size = GetLength();

			if (typeDef.minLength != -1 && (size < typeDef.minLength))
				isValid = false;

			if (typeDef.maxLength != -1 && (size > typeDef.maxLength))
				isValid = false;
		}

		// Validate custom types
		if (!CustomDataType::ValidateCustomType(typeDef, value.value()))
			isValid = false;
	}

	return isValid;
}
