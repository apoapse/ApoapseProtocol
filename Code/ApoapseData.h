#pragma once
#include "TypeDefs.hpp"
#include <any>
#include <optional>

enum class DataFieldType
{
	undefined,
	integer,
	boolean,
	text,
	blob,
	uuid,
	username
};

class ApoapseData
{
	struct DataField
	{
		std::string name;
		DataFieldType type;
		bool isRequired = true;
		bool isDataUnique = false;

		bool usedInServerDb = false;
		bool usedInClientDb = false;
		bool usedInCommad = false;

		std::optional<std::any> value;

		template <typename T>
		T GetValue()
		{
			std::any_cast<T>(value.value());
		}

		template <typename T>
		T GetValueOr(T optional)
		{
			std::any_cast<T>(value.value_or(optional));
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
	};

	std::vector<DataStructure> m_registeredDataStructures;

public:
	void ReadRegisteredDataStructures(const std::string& jsonStr);
	static DataFieldType GetTypeByTypeName(const std::string& typeStr);
};