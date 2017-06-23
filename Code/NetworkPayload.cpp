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

NetworkPayload::NetworkPayload()
{
	
}


void NetworkPayload::ReadHeaderFirstPart(Range<std::array<byte, READ_BUFFER_SIZE>>& data)
{
	headerInfo = NetworkMessageHeader();

	headerInfo->command = (IdsCommand)FromBytes<UInt16>(data, Endianness::BIG_ENDIAN);
	data.Consume(sizeof(UInt16));

	switch (data[0])
	{
	case 0x00:
		headerInfo->payloadLength = ReadExplicitLength<UInt16>(data);
		break;

	case 0x01:
		headerInfo->payloadLength = ReadExplicitLength<UInt32>(data);
		break;

	case 0x02:
		headerInfo->payloadLength = ReadExplicitLength<UInt64>(data);
		break;

	default:
		headerInfo->payloadLength = static_cast<UInt64>(data[0]);
		data.Consume(1);
		break;
	}
}

void NetworkPayload::ReadHeaderPayloadLength(Range<std::array<byte, READ_BUFFER_SIZE>>& data)
{
	switch (data[0])
	{
	case 0x00:
		headerInfo->payloadLength = ReadExplicitLength<UInt16>(data);
		break;

	case 0x01:
		headerInfo->payloadLength = ReadExplicitLength<UInt32>(data);
		break;

	case 0x02:
		headerInfo->payloadLength = ReadExplicitLength<UInt64>(data);
		break;
	}

	data.Consume(m_payloadLengthIndicatorSize.value());
}

void NetworkPayload::Insert(Range<std::array<byte, READ_BUFFER_SIZE>>& range)
{
	// In header construction phase
	if (!headerInfo)
	{
		if (!m_headerData && !m_payloadLengthIndicatorSize)
		{
			m_headerData = std::vector<byte>();

			const size_t upperBound = (range.Size() >= HEADER_MIN_LENGTH) ? HEADER_MIN_LENGTH : range.Size();
			m_headerData->insert(m_headerData->begin(), range.begin(), range.begin() + upperBound);
		}

		if (!m_payloadLengthIndicatorSize && m_headerData->size() >= HEADER_MIN_LENGTH)
			ReadHeaderFirstPart(range);

		if (m_payloadLengthIndicatorSize)
		{
			const size_t upperBound = (range.Size() > m_payloadLengthIndicatorSize.value()) ? m_payloadLengthIndicatorSize.value() : range.Size();
			m_headerData->insert(m_headerData->begin(), range.begin(), range.begin() + upperBound);
			range.Consume(upperBound);

			if (m_headerData->size() == m_payloadLengthIndicatorSize.value())
			{
				ReadHeaderPayloadLength(range);

				m_payloadLengthIndicatorSize.reset();
				m_headerData.reset();
			}
		}
	}

	if (headerInfo)
	{
		const size_t bytesLeft = BytesLeft();
		const size_t upperBound = (range.Size() < bytesLeft) ? range.Size() : bytesLeft;

		payloadData.insert(payloadData.begin(), range.begin(), range.begin() + upperBound);
		range.Consume(upperBound);
	}
}

size_t NetworkPayload::BytesLeft() const
{
	if (!headerInfo)
	{
		// In header construction phase
		ASSERT(m_headerData);

		if (m_payloadLengthIndicatorSize)
			return m_payloadLengthIndicatorSize.value() - m_headerData->size();
		else
			return HEADER_MIN_LENGTH - m_headerData->size();
	}
	else
	{
		auto val = (Int64)headerInfo->payloadLength - (Int64)payloadData.size();
		ASSERT_MSG(val >= 0, "The number of bytes left can't be negative");

		return val;
	}
}
