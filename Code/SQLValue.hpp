#pragma once
#include "TypeDefs.hpp"
#include <string>
#include <vector>
#include <variant>
#include <string>
#include "Maths.hpp"

enum class ValueType
{
	UNSUPPORTED,
	TEXT,
	INT,
	INT_64,
	BYTE_ARRAY
};

class SQLValue
{
	std::variant<std::string, int, Int64, std::vector<byte>> m_data;
	ValueType m_type;

public:
	template <typename T>
	SQLValue(const T& value, ValueType type)
		: m_data(value),
		m_type(type)
	{
	}

	ValueType GetType() const
	{
		return m_type;
	}

	template <typename T>
	static ValueType GenerateType()
	{
		if (std::type_index(typeid(T)) == std::type_index(typeid(std::string)))
			return ValueType::TEXT;

		else if (std::type_index(typeid(T)) == std::type_index(typeid(int)))
			return ValueType::INT;

		else if (std::type_index(typeid(T)) == std::type_index(typeid(Int64)))
			return ValueType::INT_64;

		if (std::type_index(typeid(T)) == std::type_index(typeid(std::vector<byte>)))
			return ValueType::BYTE_ARRAY;

		else
			return ValueType::UNSUPPORTED;
	}

	int GetInt32() const
	{
		if (GetType() == ValueType::INT)
			return std::get<int>(m_data);

		else if (GetType() == ValueType::INT_64)
		{
			const auto val = std::get<Int64>(m_data);

			if (!CanFit<int>(val))
				throw std::overflow_error("The requested value cannot fit into a Int32");

			return static_cast<int>(val);
		}

		else
			throw std::bad_typeid();
	}

	Int64 GetInt64() const
	{
		if (GetType() == ValueType::INT_64)
			return std::get<Int64>(m_data);

		else if (GetType() == ValueType::INT)
			return static_cast<Int64>(GetInt32());

		else
			throw std::bad_typeid();
	}

	const std::string& GetText() const
	{
		if (GetType() == ValueType::TEXT)
			return std::get<std::string>(m_data);
		else
			throw std::bad_typeid();
	}

	bool GetBoolean() const
	{
		if (GetType() == ValueType::INT_64 || GetType() == ValueType::INT)
			return (GetInt32() == 1);
		else
			throw std::bad_typeid();
	}

	const std::vector<byte>& GetByteArray() const
	{
		if (GetType() == ValueType::BYTE_ARRAY)
			return std::get<std::vector<byte>>(m_data);
		else
			throw std::bad_typeid();
	}
};