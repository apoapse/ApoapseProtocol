#include "stdafx.h"
#include "Common.h"
#include "DataStructure.h"
#include "SQLQuery.h"
#include "Json.hpp"

DataField& DataStructure::GetField(const std::string& fieldName)
{
	auto res = std::find_if(fields.begin(), fields.end(), [&fieldName](DataField& field)
	{
		return (field.name == fieldName);
	});

	if (res == fields.end())
		throw std::exception("The requested field do not exist");

	return *res;
}

bool DataStructure::FieldExist(const std::string& fieldName)
{
	return std::count_if(fields.begin(), fields.end(), [&fieldName](const DataField& field)
	{
		return (field.name == fieldName);
	});
}

MessagePackSerializer DataStructure::GetMessagePackFormat(bool includeAllFields/* = false*/)
{
	MessagePackSerializer ser;

	for (auto& field : fields)
	{
		if (!field.HasValue())
			continue;

		if (!includeAllFields && !field.usedInCommand)
			continue;
		
		if (field.basicType == DataFieldType::boolean)
			ser.UnorderedAppend<bool>(field.name, field.GetValue<bool>());

		else if (field.basicType == DataFieldType::byte_blob)
			ser.UnorderedAppend<ByteContainer>(field.name, field.GetValue<ByteContainer>());

		else if (field.basicType == DataFieldType::integer)
			ser.UnorderedAppend<Int64>(field.name, field.GetValue<Int64>());

		else if (field.basicType == DataFieldType::text)
			ser.UnorderedAppend<std::string>(field.name, field.GetValue<std::string>());
	}

	return ser;
}

void DataStructure::SendUISignal(const std::string& signalName)
{
	ASSERT(global->isClient);
	JsonHelper json;

	for (auto& field : fields)
	{
		if (field.usedInClientUI && field.HasValue())
		{
			if (field.basicType == DataFieldType::boolean)
				json.Insert(field.name, field.GetValue<bool>());

			//if (field.basicType == DataFieldType::byte_blob)
			//	json.Insert(field.name, field.GetValue<ByteContainer>());

			if (field.basicType == DataFieldType::integer)
				json.Insert(field.name, field.GetValue<Int64>());

			if (field.basicType == DataFieldType::text)
				json.Insert(field.name, field.GetValue<std::string>());
		}
	}

	global->htmlUI->SendSignal(signalName, json.Generate());
}

DbId DataStructure::GetDbId()
{
	if (!dbId.has_value())
	{
		DataField primaryField = GetPrimaryField().value();

		SQLQuery query(*global->database);
		query << SELECT << "id" << FROM << GetDBTableName().c_str() << WHERE << primaryField.name.c_str() << EQUALS << primaryField.GetSQLValue();
		auto res = query.Exec();

		if (res && res.RowCount() > 0)
		{
			dbId = res[0][0].GetInt64();
		}
		else
		{
			// If the item is not already registered, we predict the id that will be used once saved
			SQLQuery query(*global->database);
			query << SELECT << "COUNT(*)" FROM << GetDBTableName().c_str();
			auto res = query.Exec();

			dbId = (res[0][0].GetInt64() + 1);
		}
	}

	return dbId.value();
}

void DataStructure::SetDbId(DbId id)
{
	dbId = id;
}

void DataStructure::SaveToDatabase()
{
	const int nbFieldToSave = std::count_if(fields.begin(), fields.end(), [](const DataField& field)
	{
		return (field.usedInClientDb == global->isClient || field.usedInServerDb && global->isServer);
	});

	if (nbFieldToSave == 0)
	{
		LOG << LogSeverity::error << "Trying to save data of type " << name << " but none of the fields are set to be saved on the database";
		return;
	}

	std::vector<DataField*> fieldsToSave;
	for (auto& field : fields)
	{
		if (ApoapseData::AllowDatabaseStorage(field) && field.HasValue())
			fieldsToSave.push_back(&field);
	}

	auto& primaryField = GetPrimaryField();

	SQLQuery query(*global->database);

	if (primaryField && IsAlreadyRegisteredOnDatabase(primaryField.value()))
	{
		// Update
		query << UPDATE << GetDBTableName().c_str() << SET;

		for (size_t i = 0; i < fieldsToSave.size(); i++)
		{
			auto& field = fieldsToSave.at(i);
			query << field->name.c_str() << "=" << field->GetSQLValue();

			if (i != fieldsToSave.size() - 1)
				query << ",";
		}

		query << WHERE << primaryField.value().name.c_str() << EQUALS << primaryField.value().GetSQLValue();
		query.Exec();

		LOG << LogSeverity::verbose << "Udpated entry on the database with data from the datastructure " << name;
	}
	else
	{
		// Insert
		query << INSERT_INTO << GetDBTableName().c_str() << " (";

		for (size_t i = 0; i < fieldsToSave.size(); i++)
		{
			query << fieldsToSave.at(i)->name.c_str();

			if (i != fieldsToSave.size() - 1)
				query << ",";
		}

		query << ")" << VALUES << "(";

		for (size_t i = 0; i < fieldsToSave.size(); i++)
		{
			query << fieldsToSave.at(i)->GetSQLValue();

			if (i != fieldsToSave.size() - 1)
				query << ",";
		}

		query << ")";
		query.Exec();

		LOG << LogSeverity::verbose << "Added new entry on the database with data from the datastructure " << name;
	}
}

inline std::string DataStructure::GetDBTableName() const
{
	return name + "s"; //  // We set the table name as plurial
}

void DataStructure::AutoFillFieldsIfRequested()
{
	for (auto& field : fields)
	{
		if (field.customType.has_value() && !field.HasValue())
		{
			const auto typeDef = global->apoapseData->GetCustomTypeInfo(field.customType.value());

			if (typeDef.autoFill)
			{
				CustomDataType::AutoFill(typeDef, field);
				LOG_DEBUG << "Field " << field.name << " of data structure " << name << " has been auto filled";
			}
		}
	}
}

bool DataStructure::IsAlreadyRegisteredOnDatabase(DataField& primaryField)
{
	SQLQuery query(*global->database);
	query << SELECT << primaryField.name.c_str() << FROM << GetDBTableName().c_str() << WHERE << primaryField.name.c_str() << EQUALS << primaryField.GetSQLValue();
	auto res = query.Exec();

	return (res && res.RowCount() == 1);
}

std::optional<DataField> DataStructure::GetPrimaryField()
{
	for (auto& field : fields)
	{
		if (field.HasValue() && field.isDataUnique)
			return field;
	}

	return std::optional<DataField>();
}

bool DataField::Validate() const
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

SQLValue DataField::GetSQLValue()
{
	if (basicType == DataFieldType::boolean)
		return SQLValue(GetValue<bool>(), ConvertFieldTypeToSqlType());

	else if (basicType == DataFieldType::byte_blob)
		return SQLValue(GetValue<ByteContainer>(), ConvertFieldTypeToSqlType());

	else if (basicType == DataFieldType::integer)
		return SQLValue(GetValue<Int64>(), ConvertFieldTypeToSqlType());

	else if (basicType == DataFieldType::text)
		return SQLValue(GetValue<std::string>(), ConvertFieldTypeToSqlType());

	else
		return SQLValue();
}

inline size_t DataField::GetLength() const
{
	if (basicType == DataFieldType::integer)
		return std::any_cast<Int64>(value.value());

	else if (basicType == DataFieldType::byte_blob)
		return std::any_cast<ByteContainer>(value.value()).size();

	else if (basicType == DataFieldType::text)
		return std::any_cast<std::string>(value.value()).size();

	else
		return (size_t)-1;
}

SqlValueType DataField::ConvertFieldTypeToSqlType() const
{
	switch (basicType)
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