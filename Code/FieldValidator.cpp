#include "stdafx.h"
#include "FieldValidator.h"
#include "Common.h"

bool Field::DoesFieldHaveValue(const Field& field, const MessagePackDeserializer& source)
{
	return field.fieldValueValidator->HasValue(field.name, source);
}

bool Field::ValidateField(const Field& field, bool valueExist, const MessagePackDeserializer& source)
{
	if (field.requirement == FieldRequirement::any_mendatory && !valueExist)
	{
		LOG << "The required field " << field.name << " is missing" << LogSeverity::warning;
		return false;
	}

	if (valueExist)
	{
		if (!field.fieldValueValidator->ExecValidator(field.name, source))
		{
			LOG << "The field " << field.name << " is invalid according to an user provided check function" << LogSeverity::warning;
			return false;
		}
	}

	return true;
}
