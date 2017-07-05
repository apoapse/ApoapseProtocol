#include "stdafx.h"
#include "Command.h"
#include "Common.h"
#include "NetworkPayload.h"
#include "MacroUtils.hpp"

void Command::Parse(std::shared_ptr<NetworkPayload> data)
{
	ASSERT(m_deserializedData.get() == nullptr);

	m_rawData = std::move(data);
	m_deserializedData = std::make_unique<MessagePackDeserializer>(m_rawData->payloadData);

	try
	{
		m_deserializedData->Parse();
	}
	catch (const std::exception& e)
	{
		LOG << LogSeverity::error << "Error while parsing a command of type " << (UInt16)GetInfo().command << " : " << e;
		m_isValid = false;
	}

	AutoValidate();
}

bool Command::IsValid() const
{
	return m_isValid;
}

void Command::AutoValidate()
{
	m_isValid = true;
	const auto fields = GetInfo().fields;

	for (const auto& field : fields)
	{
		ASSERT_MSG(field.fieldValueValidator != nullptr, "A FIELD_VALUE_VALIDATOR or FIELD_VALUE marco is missing inside the command info fields definition");
		bool valueExist = false;

		try
		{
			valueExist = field.fieldValueValidator->HasValue(field.name, *m_deserializedData.get());
		}
		catch (const std::exception& e)
		{
			LOG << "Command auto validate: required field " << field.name << " on command " << (UInt16)GetInfo().command << " returned the following exception: " << e.what() << LogSeverity::warning;
			m_isValid = false;
#ifndef DEBUG
			return;
#else
			continue;
#endif // DEBUG
		}

		ValidateField(field, valueExist);
	}
}

void Command::ValidateField(const CommandField& field, bool valueExist)
{
	if (field.requirement == FieldRequirement::ANY_MENDATORY && !valueExist)
	{
		LOG << "Command auto validate: required field " << field.name << " on command " << (UInt16)GetInfo().command << " is missing" << LogSeverity::warning;
		m_isValid = false;
		return;
	}

	if (valueExist && field.fieldValueValidator->HasValidatorFunction())
	{
		if (!field.fieldValueValidator->ExecValidator(field.name, *m_deserializedData.get()))
		{
			LOG << "Command auto validate: the field " << field.name << " on command " << (UInt16)GetInfo().command << " is invalid according to an user provided check function" << LogSeverity::warning;
			return;
		}
	}
}
