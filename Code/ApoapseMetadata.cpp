#include "stdafx.h"
#include "ApoapseMetadata.h"
#include "Common.h"

ApoapseMetadata::ApoapseMetadata(const SQLValue& sqlVal, MetadataAcess acess) : m_acess(acess)
{
	m_rawData = sqlVal.GetByteArray();
}

ApoapseMetadata::ApoapseMetadata(MessagePackSerializer& ser, MetadataAcess acess)
{
	m_rawData = ser.GetMessagePackBytes();
}

ApoapseMetadata::ApoapseMetadata(const std::vector<byte>& data, MetadataAcess acess)
{
	m_rawData = data;
}

MessagePackDeserializer ApoapseMetadata::ReadData() const
{
	return MessagePackDeserializer(m_rawData);
}

std::vector<byte> ApoapseMetadata::GetRawData() const
{
	return m_rawData;
}

std::vector<byte> ApoapseMetadata::GetRawDataForDb() const
{
	return m_rawData;
}

bool ApoapseMetadata::ValidateMetadataFields(const ApoapseMetadata& metadata, const std::vector<Field>& fieldValidators)
{
	MessagePackDeserializer source = metadata.ReadData();

	for (const auto& field : fieldValidators)
	{
		bool doesValueExist = false;

		try
		{
			doesValueExist = Field::DoesFieldHaveValue(field, source);
		}
		catch (const std::exception& e)
		{
			return false;
		}

		const bool isInvalidField = !Field::ValidateField(field, doesValueExist, source);
	}

	return true;
}
