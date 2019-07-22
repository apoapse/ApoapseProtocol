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
	byte_blob
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
			throw std::exception("DataField:SetValue no ICustomDataType converter defined for this base type");
	}

	bool HasValue() const
	{
		return value.has_value();
	}

	bool Validate() const;

	SQLValue GetSQLValue();
	SqlValueType ConvertFieldTypeToSqlType() const;

private:
	size_t GetLength() const;
};

class DataStructure
{
	std::optional<DbId> dbId;

public:
	std::string name;
	std::vector<DataField> fields;
	bool isValid = true;

	DataField& GetField(const std::string& fieldName);

	MessagePackSerializer GetMessagePackFormat();

	void SendUISignal(const std::string& signalName);
	void SaveToDatabase();
	std::string GetDBTableName() const;
	void AutoFillFieldsIfRequested();

	DbId GetDbId();
	void SetDbId(DbId id);

private:
	bool IsAlreadyRegisteredOnDatabase(DataField& primaryField);
	DataField& GetPrimaryField();
};