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
#include <any>
#include <optional>
#include "SQLValueType.hpp"
#include "SQLValue.hpp"
#include "CustomDataType.h"
class NetworkPayload;
class JsonHelper;

enum class ReadPermission
{
	none,
	author,
	usergroup,
	all
};

enum class DataFieldType
{
	undefined,
	integer,
	boolean,
	text,
	byte_blob,
	data_array
};


struct CustomFieldType
{
	std::string name;
	DataFieldType underlyingType = DataFieldType::undefined;
	int minLength = -1;
	int maxLength = -1;
	bool autoFill = false;
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
	bool usedInCommand = false;
	bool usedInClientUI = false;

	ReadPermission readPermission = ReadPermission::none;

	std::optional<std::any> value;
	std::optional<std::vector<class DataStructure>> dataArray;

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

	std::vector<class DataStructure>& GetDataArray()
	{
		return dataArray.value();
	}

	void SetValue(Int64 val)
	{
		value = val;
	}

	void SetValue(bool val)
	{
		value = val;
	}

	inline void SetValue(const char* val)
	{
		value = std::string(val);
	}

	void SetValue(const std::string& val)
	{
		value = val;
	}

	void SetValue(const ByteContainer& val)
	{
		value = val;
	}

	void SetValue(const ICustomDataType& val)
	{
		if (basicType == DataFieldType::text)
			value = val.GetStr();

		else if (basicType == DataFieldType::byte_blob)
			value = val.GetBytes();

		else
			throw std::runtime_error("DataField:SetValue no ICustomDataType converter defined for this base type");
	}

	void SetValue(std::vector<class DataStructure>& structArray)
	{
		dataArray = structArray;
	}

	bool HasValue() const
	{
		if (basicType == DataFieldType::data_array)
			return dataArray.has_value();
		else
			return value.has_value();
	}

	bool Validate() const;
	void SaveToDatabase(class DataStructure& dat, DbId useId = 0);

	SQLValue GetSQLValue();
	SqlValueType ConvertFieldTypeToSqlType() const;

private:
	size_t GetLength() const;
};

class DataStructure
{
	std::optional<DbId> dbId;

public:
	DataStructure() = default;
	
	std::string name;
	std::vector<DataField> fields;
	bool isValid = true;

	DataField& GetField(const std::string& fieldName);
	bool FieldExist(const std::string& fieldName);

	MessagePackSerializer GetMessagePackFormat(bool includeAllFields = false);

	void SendUISignal(const std::string& signalName);
	void SaveToDatabase();
	std::string GetDBTableName() const;
	void AutoFillFieldsIfRequested();

	DbId GetDbId();
	void SetDbId(DbId id);

	std::optional<DataField> GetPrimaryField();
private:
	bool IsAlreadyRegisteredOnDatabase(DataField& primaryField);
};