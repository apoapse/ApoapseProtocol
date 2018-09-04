#pragma once
#include "SQLPackagedResult.hpp"
#include "MessagePack.hpp"
#include "FieldValidator.h"

#define BIT(x)    (1u << (x))
enum MetadataAcess
{
	undefined		= BIT(0),
	self			= BIT(1),
	usergroup		= BIT(2),
	all				= BIT(3),
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