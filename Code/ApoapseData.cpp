#include "stdafx.h"
#include "Common.h"
#include "ApoapseData.h"
#include "Json.hpp"

ApoapseData::ApoapseData(const std::string& dataSchemeJson)
{
	JsonHelper json(dataSchemeJson);

	for (const auto& dser : json.ReadFieldArray<JsonHelper>("data_structures"))
	{
		DataStructure dataStructure;
		dataStructure.name = dser.ReadFieldValue<std::string>("name").value();

		for (const auto& fieldDser : dser.ReadFieldArray<JsonHelper>("fields"))
		{
			DataField field;

			field.name = fieldDser.ReadFieldValue<std::string>("name").value();
			field.type = GetTypeByTypeName(fieldDser.ReadFieldValue<std::string>("type").value());
			field.isDataUnique = fieldDser.ReadFieldValue<bool>("unique").value_or(false);
			field.usedInServerDb = fieldDser.ReadFieldValue<bool>("uses.server_storage").value_or(false);
			field.usedInClientDb = fieldDser.ReadFieldValue<bool>("uses.client_storage").value_or(false);
			field.usedInCommad = fieldDser.ReadFieldValue<bool>("uses.command").value_or(false);

			dataStructure.fields.push_back(field);
		}

		m_registeredDataStructures.push_back(dataStructure);
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

	return DataFieldType::undefined;
}
