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