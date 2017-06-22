#include "stdafx.h"
#include "NetworkPayload.h"
#include "ByteUtils.hpp"
#include "Diagnostic.hpp"

template <typename T>
UInt64 ReadExplicitLength(Range<std::array<byte, READ_BUFFER_SIZE>>& data)
{
	data.Consume(1);	// Consume the byte defining the type of which the length is encoded

	if (data.Size() < sizeof(T))
		throw std::exception("Not enough bytes to make the requested type");

	auto length = (UInt64)FromBytes<T>(data, Endianness::BIG_ENDIAN);

	data.Consume(sizeof(T));
	return length;
}

NetworkPayload::NetworkPayload(Range<std::array<byte, READ_BUFFER_SIZE>>& workingData)
{
	if (workingData.Size() >= 3)
		headerInfo = ReadHeader(workingData);
}

size_t NetworkPayload::BytesLeft() const
{
	auto nb = (Int64)headerInfo.payloadLength - (Int64)payloadData.size();
	ASSERT_MSG(nb >= 0, "The number of bytes left cannot be negative");

	return (size_t)nb;
}

NetworkMessageHeader NetworkPayload::ReadHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& data)
{
	if (data.Size() < 3)
		throw std::exception("A Apoapse payload header should be 3 bytes minimum");

	NetworkMessageHeader headerInfo;

	headerInfo.command = (IdsCommand)FromBytes<UInt16>(data, Endianness::BIG_ENDIAN);
	data.Consume(sizeof(UInt16));

	switch (data[0])
	{
	case 0x00:
		headerInfo.payloadLength = ReadExplicitLength<UInt16>(data);
		break;

	case 0x01:
		headerInfo.payloadLength = ReadExplicitLength<UInt32>(data);
		break;

	case 0x02:
		headerInfo.payloadLength = ReadExplicitLength<UInt64>(data);
		break;

	default:
		headerInfo.payloadLength = static_cast<UInt64>(data[0]);
		data.Consume(1);
		break;
	}

	return headerInfo;
}
