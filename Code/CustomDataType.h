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
#include <any>
#include "TypeDefs.hpp"
#include "ICustomDataType.hpp"
#include "Uuid.h"
#include "Username.h"
#include "DateTimeUtils.h"
struct CustomFieldType;
struct DataField;

namespace CustomDataType
{
	bool ValidateCustomType(const CustomFieldType& typeDef, const std::any& value);
	void AutoFill(const CustomFieldType& typeDef, DataField& field);

	template <typename T>
	T ConvertToCustomType(const std::any& value)
	{
		if constexpr (std::is_same<T, Uuid>::value)
		{
			return Uuid(std::any_cast<ByteContainer>(value));
		}
		else if constexpr (std::is_same<T, Username>::value)
		{
			return Username(std::any_cast<ByteContainer>(value));
		}
		else if constexpr (std::is_same<T, DateTimeUtils::UTCDateTime>::value)
		{
			return DateTimeUtils::UTCDateTime(std::any_cast<std::string>(value));
		}
		else
		{
			//LOG_DEBUG << "The custom field type " << typeName << "do not have a type converter. Using the provided base type instead.";
			return std::any_cast<T>(value);
		}
	}
}