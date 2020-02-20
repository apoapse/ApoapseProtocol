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
#include <string>
#include <vector>
#include <variant>
#include <string>
#include "Maths.hpp"
#include "SQLValueType.hpp"

class SQLValue
{
	std::variant<std::string, int, Int64, std::vector<byte>> m_data;
	SqlValueType m_type = SqlValueType::UNSUPPORTED;

public:
	SQLValue() = default;

	template <typename T>
	SQLValue(const T& value, SqlValueType type)
		: m_type(type)
		, m_data(value)
	{
	}

	SQLValue(const SQLValue& sqlValue)
		: m_type(sqlValue.m_type)
		, m_data(sqlValue.m_data)
	{
	}

	SqlValueType GetType() const
	{
		return m_type;
	}

	template <typename T>
	static SqlValueType GenerateType()
	{
		if constexpr (std::is_same<T, std::string>::value)
			return SqlValueType::TEXT;

		else if constexpr (std::is_same<T, int>::value)
			return SqlValueType::INT;

		else if constexpr (std::is_same<T, Int64>::value)
			return SqlValueType::INT_64;

		else if constexpr (std::is_same<T, std::vector<byte>>::value)
			return SqlValueType::BYTE_ARRAY;

		else
			return SqlValueType::UNSUPPORTED;
	}

	int GetInt32() const
	{
		if (GetType() == SqlValueType::INT)
			return std::get<int>(m_data);

		else if (GetType() == SqlValueType::INT_64)
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
		if (GetType() == SqlValueType::INT_64)
			return std::get<Int64>(m_data);

		else if (GetType() == SqlValueType::INT)
			return static_cast<Int64>(GetInt32());

		else
			throw std::bad_typeid();
	}

	std::string GetText() const
	{
		if (GetType() == SqlValueType::TEXT)
		{
			return std::get<std::string>(m_data);
		}
		else if (GetType() == SqlValueType::BYTE_ARRAY)
		{
			const auto bytes = std::get<std::vector<byte>>(m_data);
			return std::string(bytes.begin(), bytes.end());
		}
		else
			throw std::bad_typeid();
	}

	bool GetBoolean() const
	{
		if (GetType() == SqlValueType::INT_64 || GetType() == SqlValueType::INT)
			return (GetInt32() == 1);
		else
			throw std::bad_typeid();
	}

	std::vector<byte> GetByteArray() const
	{
		if (GetType() == SqlValueType::BYTE_ARRAY)
		{
			return std::get<std::vector<byte>>(m_data);
		}
		else if (GetType() == SqlValueType::TEXT)
		{
			const std::string text = std::get<std::string>(m_data);
			return std::vector<byte>(text.begin(), text.end());
		}
		else
			throw std::bad_typeid();
	}
};