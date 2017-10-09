#include "stdafx.h"
#include "Command.h"
#include "Common.h"
#include "NetworkPayload.h"
#include "MacroUtils.hpp"
#include "GenericConnection.h"

void Command::Parse(std::shared_ptr<NetworkPayload> payload)
{
	ASSERT(m_deserializedData.get() == nullptr);

	try
	{
		const auto range = payload->GetPayloadContent();
		auto data = std::vector<byte>(range.begin(), range.end());
		ASSERT(data.size() == payload->headerInfo->payloadLength);

		m_deserializedData = std::make_unique<MessagePackDeserializer>(std::move(data));

		AutoValidate();
	}
	catch (const std::exception& e)
	{
		LOG << LogSeverity::error << "Error while parsing a command of type " << (UInt16)GetInfo().command << " : " << e;
		m_isValid = false;
	}
}

bool Command::IsValid() const
{
	return m_isValid;
}

void Command::Process(const ServerConnection&)
{

}

void Command::Process(const User&, const ServerConnection&)
{

}

void Command::Process(const ClientConnection&)
{

}

void Command::Send(INetworkSender& destination, TCPConnection* excludedConnection /*= nullptr*/)
{
	ASSERT(m_serializedData.has_value());
	Send(m_serializedData.value(), destination, excludedConnection);
}

void Command::Send(MessagePackSerializer& data, INetworkSender& destination, TCPConnection* excludedConnection/* = nullptr*/)
{
	auto bytes = std::vector<byte>(NetworkPayload::headerLength + data.GetMessagePackBytes().size());
	bytes.insert(bytes.begin() + NetworkPayload::headerLength, data.GetMessagePackBytes().begin(), data.GetMessagePackBytes().end());

	auto payload = std::make_unique<NetworkPayload>(GetInfo().command, std::move(bytes));
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
			valueExist = DoesFieldHaveValue(field);
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

bool Command::DoesFieldHaveValue(const CommandField &field) const
{
	return field.fieldValueValidator->HasValue(field.name, *m_deserializedData);
}

bool Command::ValidateField(const CommandField& field, bool valueExist)
{
	if (field.requirement == FieldRequirement::any_mendatory && !valueExist)
	{
		LOG << "Command auto validate: required field " << field.name << " on command " << (UInt16)GetInfo().command << " is missing" << LogSeverity::warning;
		return false;
	}

	if (valueExist)
	{
		if (!field.fieldValueValidator->ExecValidator(field.name, *m_deserializedData))
		{
			LOG << "Command auto validate: the field " << field.name << " on command " << (UInt16)GetInfo().command << " is invalid according to an user provided check function" << LogSeverity::warning;
			return false;
		}
	}

	return true;
}
