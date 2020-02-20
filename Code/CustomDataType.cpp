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

#include "stdafx.h"
#include "Common.h"
#include "CustomDataType.h"
#include "DateTimeUtils.h"
#include "Uuid.h"
#include "Username.h"
#include "DataStructure.h"

bool CustomDataType::ValidateCustomType(const CustomFieldType& typeDef, const std::any& value)
{
	if (typeDef.name == "uuid")
	{
		if (!Uuid::IsValid(std::any_cast<ByteContainer>(value)))
		{
			LOG << LogSeverity::warning << "Uuid invalid";
			return false;
		}
	}
	else if (typeDef.name == "username")
	{
		if (!Username::IsValid(std::any_cast<ByteContainer>(value)))
		{
			LOG << LogSeverity::warning << "Username invalid";
			return false;
		}
	}
	else if (typeDef.name == "datetime")
	{

		if (!DateTimeUtils::UTCDateTime::ValidateFormat(std::any_cast<std::string>(value)))
		{
			LOG << LogSeverity::warning << "UTC datetime invalid";
			return false;
		}
	}

	return true;
}

void CustomDataType::AutoFill(const CustomFieldType& typeDef, DataField& field)
{
	field.SetValue(Uuid::Generate());
}