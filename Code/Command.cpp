#include "stdafx.h"
#include "Command.h"
#include "Common.h"
#include "NetworkPayload.h"
#include "MacroUtils.hpp"
#include "GenericConnection.h"

void Command::Parse(std::shared_ptr<NetworkPayload> data)
{
	ASSERT(m_deserializedData.get() == nullptr);

	try
	{
		m_deserializedData = std::make_unique<MessagePackDeserializer>(data->payloadData);
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

void Command::Process(const GenericConnection& sender)
{
	ASSERT_MSG(false, "This function should never be called directly, it should by overloaded by the command class");
}

void Command::Process(const User& sender, const GenericConnection& senderConnection)
{
	ASSERT_MSG(false, "This function should never be called directly, it should by overloaded by the command class");
}

void Command::Send(MessagePackSerializer& data, INetworkSender& destination, TCPConnection* excludedConnection/* = nullptr*/)
{
	auto payload = std::make_unique<NetworkPayload>(GetInfo().command, std::move(data.GetMessagePackBytes()));
	destination.Send(std::move(payload), excludedConnection);
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
			valueExist = DoesFieldHasValue(field);
		}
		catch (const std::exception& e)
		{
			LOG << "Command auto validate: required field " << field.name << " on command " << (UInt16)GetInfo().command << " returned the following exception: " << e << LogSeverity::warning;
			m_isValid = false;
#ifndef DEBUG
			return;
#else
			continue;
#endif // DEBUG
		}

		const bool IsInvalidField = !ValidateField(field, valueExist);

		if (IsInvalidField)
		{			
			m_isValid = false;
#ifndef DEBUG
			return;
#else
			continue;
#endif // DEBUG
		}
	}
}

bool Command::DoesFieldHasValue(const CommandField &field) const
{
	return field.fieldValueValidator->HasValue(field.name, *m_deserializedData.get());
}

bool Command::ValidateField(const CommandField& field, bool valueExist)
{
	if (field.requirement == FieldRequirement::any_mendatory && !valueExist)
	{
		LOG << "Command auto validate: required field " << field.name << " on command " << (UInt16)GetInfo().command << " is missing" << LogSeverity::warning;
		return false;
	}

	if (valueExist && field.fieldValueValidator->HasValidatorFunction())
	{
		if (!field.fieldValueValidator->ExecValidator(field.name, *m_deserializedData.get()))
		{
			LOG << "Command auto validate: the field " << field.name << " on command " << (UInt16)GetInfo().command << " is invalid according to an user provided check function" << LogSeverity::warning;
			return false;
		}
	}

	return true;
}
