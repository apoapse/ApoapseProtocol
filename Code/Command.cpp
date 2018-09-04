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
		m_networkPayload = std::vector<byte>(range.begin(), range.end());	// The command need ownership of the network payload data. #TODO Make the NetworkPayload unique_ptr and avoid creating a vector, duplicating the data like we do currently
		ASSERT(m_networkPayload.size() == payload->headerInfo->payloadLength);

		m_deserializedData = std::make_unique<MessagePackDeserializer>(m_networkPayload);

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

void Command::Process(ServerConnection&)
{

}

void Command::Process(User&, ServerConnection&)
{

}

void Command::Process(ClientConnection&)
{

}

void Command::Send(INetworkSender& destination, TCPConnection* excludedConnection /*= nullptr*/)
{
	ASSERT(m_serializedData.has_value());
	Send(m_serializedData.value(), destination, excludedConnection);
}

void Command::Send(MessagePackSerializer& data, INetworkSender& destination, TCPConnection* excludedConnection/* = nullptr*/)
{
	const auto& msgPackBytes = data.GetMessagePackBytes();
	auto bytes = std::vector<byte>(NetworkPayload::headerLength + msgPackBytes.size());

	std::copy(msgPackBytes.begin(), msgPackBytes.end(), bytes.begin() + NetworkPayload::headerLength);

	auto payload = std::make_shared<NetworkPayload>(GetInfo().command, std::move(bytes));
	destination.Send(payload, excludedConnection);
}

void Command::Propagate(INetworkSender& destination, TCPConnection* excludedConnection /*= nullptr*/)
{
	LOG << "The command " << static_cast<int>(GetInfo().command) << " is being propagated to " << destination.GetEndpointStr();
	
	Send(m_deserializedData->ToSerializer(), destination, excludedConnection);
}

void Command::Propagate(MessagePackSerializer ser, INetworkSender& destination, TCPConnection* excludedConnection /*= nullptr*/)
{
	LOG << "The command " << static_cast<int>(GetInfo().command) << " is being propagated to " << destination.GetEndpointStr();

	Send(ser, destination, excludedConnection);
}

const MessagePackDeserializer& Command::GetFieldsData() const
{
	return *m_deserializedData;
}

ApoapseMetadata Command::GetMetadataField(MetadataAcess metadataType)
{
	switch (metadataType)
	{
		case self:
			return ApoapseMetadata(GetFieldsData().GetValue<ByteContainer>("metadata_self"), MetadataAcess::self);

		case usergroup:
			return ApoapseMetadata(GetFieldsData().GetValue<ByteContainer>("metadata_usergroup"), MetadataAcess::usergroup);

		case all:
			return ApoapseMetadata(GetFieldsData().GetValue<ByteContainer>("metadata_all"), MetadataAcess::all);

		default:
			return ApoapseMetadata();
	}
}

void Command::AutoValidate()
{
	m_isValid = true;

	if (!ValidateMetadataFields())
	{
		LOG << LogSeverity::error << "Command auto validate: at least a required metadata field does not exist";
		m_isValid = false;
		return;
	}

	const auto fields = GetInfo().fields;

	for (const auto& field : fields)
	{
		ASSERT_MSG(field.fieldValueValidator != nullptr, "A FIELD_VALUE_VALIDATOR or FIELD_VALUE marco is missing inside the command info fields definition");
		bool valueExist = false;

		try
		{
			valueExist = Field::DoesFieldHaveValue(field, *m_deserializedData);
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

		const bool isInvalidField = !Field::ValidateField(field, valueExist, *m_deserializedData);

		if (isInvalidField)
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

bool Command::ValidateMetadataFields() const
{
	const int metadataTypes = GetInfo().metadataTypes;

	if (metadataTypes == 0)
		return true;

	if (metadataTypes & MetadataAcess::self)
	{
		if (GetFieldsData().GetValueOptional<ByteContainer>("metadata_self").has_value())
		{
			if (!ApoapseMetadata::ValidateMetadataFields(ApoapseMetadata(GetFieldsData().GetValue<ByteContainer>("metadata_self"), MetadataAcess::self), GetInfo().metadataSelfFields))
				return false;
		}
		else
		{
			return false;
		}
	}

	if (metadataTypes & MetadataAcess::usergroup)
	{
		if (GetFieldsData().GetValueOptional<ByteContainer>("metadata_usergroup").has_value())
		{
			if (!ApoapseMetadata::ValidateMetadataFields(ApoapseMetadata(GetFieldsData().GetValue<ByteContainer>("metadata_usergroup"), MetadataAcess::usergroup), GetInfo().metadataUsergroupFields))
				return false;
		}
		else
		{
			return false;
		}
	}

	if (metadataTypes & MetadataAcess::all)
	{
		if (GetFieldsData().GetValueOptional<ByteContainer>("metadata_all").has_value())
		{
			if (!ApoapseMetadata::ValidateMetadataFields(ApoapseMetadata(GetFieldsData().GetValue<ByteContainer>("metadata_all"), MetadataAcess::all), GetInfo().metadataAllFields))
				return false;
		}
		else
		{
			return false;
		}
	}

	return true;
}