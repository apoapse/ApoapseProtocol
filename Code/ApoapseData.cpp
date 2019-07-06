#include "stdafx.h"
#include "Common.h"
#include "ApoapseData.h"
#include "Json.hpp"
#include "SQLQuery.h"
#include "DateTimeUtils.h"
#include "Uuid.h"
#include "Username.h"
#include "NetworkPayload.h"
#include "CommandsManagerV2.h"

ApoapseData::ApoapseData(const std::string& dataSchemeJson)
{
	JsonHelper json(dataSchemeJson);

	for (const auto& dser : json.ReadFieldArray<JsonHelper>("data_structures"))
	{
		DataStructureDef dataStructure;
		dataStructure.name = dser.ReadFieldValue<std::string>("name").value();

		for (const auto& fieldDser : dser.ReadFieldArray<JsonHelper>("fields"))
		{
			DataField field;

			field.name = fieldDser.ReadFieldValue<std::string>("name").value();
			field.type = GetTypeByTypeName(fieldDser.ReadFieldValue<std::string>("type").value());
			field.isRequired = fieldDser.ReadFieldValue<bool>("required").value_or(false);
			field.isDataUnique = fieldDser.ReadFieldValue<bool>("unique").value_or(false);
			field.usedInServerDb = fieldDser.ReadFieldValue<bool>("uses.server_storage").value_or(false);
			field.usedInClientDb = fieldDser.ReadFieldValue<bool>("uses.client_storage").value_or(false);
			field.usedInCommad = fieldDser.ReadFieldValue<bool>("uses.command").value_or(false);

			if (field.type != DataFieldType::undefined)
				dataStructure.fields.push_back(field);
			else
				LOG << LogSeverity::error << "Type " << fieldDser.ReadFieldValue<std::string>("type").value() << " used on the structure " << dataStructure.name << " does not exist";
		}

		m_registeredDataStructures.push_back(dataStructure);
	}

	LOG << "Registered " << m_registeredDataStructures.size() << " data structures";
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

		if (field.type == DataFieldType::boolean)
			field.value = payloadData.GetValue<bool>(field.name);

		else if (field.type == DataFieldType::byte_blob)
			field.value = payloadData.GetValue<ByteContainer>(field.name);

		if (field.type == DataFieldType::datetime)
		{
			const auto val = payloadData.GetValue<std::string>(field.name);

			if (DateTimeUtils::UTCDateTime::ValidateFormat(val))
				field.value = DateTimeUtils::UTCDateTime(val);
			else
			{
				LOG << "The datatime of the field " << field.name << " from the net payload is not valid" << LogSeverity::error;
				data.isValid = false;
			}
		}

		if (field.type == DataFieldType::integer)
			field.value = payloadData.GetValue<Int64>(field.name);

		if (field.type == DataFieldType::text)
			field.value = payloadData.GetValue<std::string>(field.name);

		if (field.type == DataFieldType::timestamp)
			field.value = payloadData.GetValue<Int64>(field.name);

		if (field.type == DataFieldType::username)
		{
			const auto val = payloadData.GetValue<ByteContainer>(field.name);

			if (Username::IsValid(val))
				field.value = Username(val);
			else
			{
				LOG << "The Username of the field " << field.name << " from the net payload is not valid" << LogSeverity::error;
				data.isValid = false;
			}
		}

		if (field.type == DataFieldType::uuid)
		{
			const auto val = payloadData.GetValue<ByteContainer>(field.name);

			if (Uuid::IsValid(val))
				field.value = Uuid(val);
			else
			{
				LOG << "The Username of the field " << field.name << " from the net payload is not valid" << LogSeverity::error;
				data.isValid = false;
			}
		}
	}

	return data;
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
	switch (field.type)
	{
	case DataFieldType::integer:
		return SqlValueType::INT_64;

	case DataFieldType::boolean:
		return SqlValueType::INT;

	case DataFieldType::byte_blob:
		return SqlValueType::BYTE_ARRAY;

	case DataFieldType::text:
		return SqlValueType::TEXT;

	case DataFieldType::datetime:
		return SqlValueType::TEXT;

	case DataFieldType::username:
		return SqlValueType::BYTE_ARRAY;

	case DataFieldType::uuid:
		return SqlValueType::BYTE_ARRAY;

	case DataFieldType::timestamp:
		return SqlValueType::INT_64;

	default:
		return SqlValueType::UNSUPPORTED;
	}
}

DataFieldType ApoapseData::GetTypeByTypeName(const std::string& typeStr)
{
	if (typeStr == "integer")
		return DataFieldType::integer;

	else if (typeStr == "bool")
		return DataFieldType::boolean;

	else if (typeStr == "text")
		return DataFieldType::text;

	else if (typeStr == "blob")
		return DataFieldType::byte_blob;

	else if (typeStr == "uuid")
		return DataFieldType::uuid;

	else if (typeStr == "username")
		return DataFieldType::username;

	else if (typeStr == "datetime")
		return DataFieldType::datetime;

	else if (typeStr == "timestamp")
		return DataFieldType::timestamp;

	return DataFieldType::undefined;
}

DataStructure ApoapseData::ReadItemFromDatabaseInternal(const std::string& name, const std::string& seachFieldName, const SQLValue& searchValue)
{
	auto& structureDef = GetStructureDefinition(name);
	//auto& fieldInfo = structureDef.GetField(seachFieldName);

	SQLQuery query(*global->database);
	query << SELECT << ALL << FROM << std::string(name + "s").c_str() << WHERE << seachFieldName.c_str() << EQUALS << searchValue;
	auto res = query.Exec();

	DataStructure data = structureDef;

	data.dbId = res[0][0].GetInt64();

	int dbValueId = 1;	// We start at id 1 to skip the id db field which is added by default 
	for (auto& field : data.fields)
	{
		if (IsStoredOnTheDatabase(field))
		{
			if (field.type == DataFieldType::boolean)
				field.value = res[0][dbValueId].GetBoolean();

			if (field.type == DataFieldType::byte_blob)
				field.value = res[0][dbValueId].GetByteArray();

			if (field.type == DataFieldType::datetime)
				field.value = DateTimeUtils::UTCDateTime(res[0][dbValueId].GetText());

			if (field.type == DataFieldType::integer)
				field.value = res[0][dbValueId].GetInt64();

			if (field.type == DataFieldType::text)
				field.value = res[0][dbValueId].GetText();

			if (field.type == DataFieldType::timestamp)
				field.value = res[0][dbValueId].GetInt64();

			if (field.type == DataFieldType::username)
				field.value = Username(res[0][dbValueId].GetByteArray());

			if (field.type == DataFieldType::uuid)
				field.value = Uuid(res[0][dbValueId].GetByteArray());

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
		if (field.type == DataFieldType::boolean)
			ser.UnorderedAppend<bool>(field.name, field.GetValue<bool>());

		if (field.type == DataFieldType::byte_blob)
			ser.UnorderedAppend<ByteContainer>(field.name, field.GetValue<ByteContainer>());

		if (field.type == DataFieldType::datetime)
			ser.UnorderedAppend<std::string>(field.name, field.GetValue<DateTimeUtils::UTCDateTime>().str());

		if (field.type == DataFieldType::integer)
			ser.UnorderedAppend<Int64>(field.name, field.GetValue<Int64>());

		if (field.type == DataFieldType::text)
			ser.UnorderedAppend<std::string>(field.name, field.GetValue<std::string>());

		if (field.type == DataFieldType::timestamp)
			ser.UnorderedAppend<Int64>(field.name, field.GetValue<Int64>());

		if (field.type == DataFieldType::username)
			ser.UnorderedAppend<ByteContainer>(field.name, field.GetValue<Username>().GetBytes());

		if (field.type == DataFieldType::uuid)
			ser.UnorderedAppend<ByteContainer>(field.name, field.GetValue<Uuid>().GetAsByteVector());
	}

	return ser;
}
