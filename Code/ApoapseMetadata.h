#pragma once
#include "SQLPackagedResult.hpp"
#include "MessagePack.hpp"
#include "FieldValidator.h"

enum MetadataAcess
{
	undefined		= 0x00,
	self			= 0x01,
	usergroup		= 0x02,
	all				= 0x03,
};

class ApoapseMetadata
{
	MetadataAcess m_acess = MetadataAcess::undefined;
	std::vector<byte> m_rawData;

public:
	ApoapseMetadata() = default;

	ApoapseMetadata(const SQLValue& sqlVal, MetadataAcess acess);			// From DB
	ApoapseMetadata(const std::vector<byte>& data, MetadataAcess acess);	// From network payload
	ApoapseMetadata(MessagePackSerializer& ser, MetadataAcess acess);		// From code

	MessagePackDeserializer ReadData() const;
	std::vector<byte> GetRawData() const;
	std::vector<byte> GetRawDataForDb() const;
	//virtual ~ApoapseMetadata();
	
	static bool ValidateMetadataFields(const ApoapseMetadata& metadata, const std::vector<Field>& fieldValidators);
private:
};