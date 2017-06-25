#include "stdafx.h"
#include "NetworkPayload.h"
#include "ByteUtils.hpp"
#include "Diagnostic.hpp"
#include "Maths.hpp"

template <typename T>
UInt64 ReadExplicitLength(Range<std::vector<byte>>& data)
{
	data.Consume(1);	// Consume the byte defining the type of which the length is encoded

	if (data.Size() < sizeof(T))
		throw std::exception("Not enough bytes to make the requested type");

	auto length = (UInt64)FromBytes<T>(data, Endianness::BIG_ENDIAN);

	data.Consume(sizeof(T));
	return length;
}

void NetworkPayload::ReadHeaderFirstPart()
{
	Range<std::vector<byte>> data(m_headerData.value(), HEADER_MIN_LENGTH);
	headerInfo = NetworkMessageHeader();

	ASSERT(m_headerData->size() >= HEADER_MIN_LENGTH);

	headerInfo->command = (IdsCommand)FromBytes<UInt16>(data, Endianness::BIG_ENDIAN);
	data.Consume(sizeof(UInt16));

	switch (data[0])
	{
	case 0x00:
		m_payloadLengthIndicatorSize = static_cast<UInt16>(sizeof(UInt16));
		break;

	case 0x01:
		m_payloadLengthIndicatorSize = static_cast<UInt16>(sizeof(UInt32));
		break;

	case 0x02:
		m_payloadLengthIndicatorSize = static_cast<UInt16>(sizeof(UInt64));
		break;

	default:
		headerInfo->payloadLength = static_cast<UInt64>(data[0]);
		m_payloadLengthIndicatorSize = 0;
		m_headerData.reset();
		break;
	}
}

void NetworkPayload::ReadHeaderPayloadLength()
{
	Range<std::vector<byte>> data(m_headerData.value());
	data.Consume(sizeof(UInt16));	// Consume the size of the command type identifier

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
}

void NetworkPayload::Insert(Range<std::array<byte, READ_BUFFER_SIZE>>& range)
{
	// In header construction phase
	if (!headerInfo)
	{
		if (m_isFirstInsert)
		{
			// Very first call
			m_headerData = std::vector<byte>();
			m_headerData->reserve(3);

			const size_t upperBound = (range.Size() >= HEADER_MIN_LENGTH) ? HEADER_MIN_LENGTH : range.Size();
			m_headerData->insert(m_headerData->end(), range.begin(), range.begin() + upperBound);
			range.Consume(upperBound);

			m_isFirstInsert = false;
		}

		if (!m_payloadLengthIndicatorSize && m_headerData->size() < HEADER_MIN_LENGTH)
		{
			// The very first bytes before calling ReadHeaderFirstPart are not all here yet
			const size_t upperBound = (range.Size() >= HEADER_MIN_LENGTH - m_headerData->size()) ? HEADER_MIN_LENGTH - m_headerData->size() : range.Size();
			m_headerData->insert(m_headerData->end(), range.begin(), range.begin() + upperBound);
			range.Consume(upperBound);
		}

		if (!headerInfo && m_headerData->size() >= HEADER_MIN_LENGTH)
		{
			ReadHeaderFirstPart();
		}

		if (m_payloadLengthIndicatorSize && headerInfo->payloadLength == 0)
		{
			const size_t upperBound = (range.Size() > m_payloadLengthIndicatorSize.value()) ? m_payloadLengthIndicatorSize.value() : range.Size();
			m_headerData->insert(m_headerData->end(), range.begin(), range.begin() + upperBound);
			range.Consume(upperBound);

			if (m_headerData->size() > m_payloadLengthIndicatorSize.value())
			{
				ReadHeaderPayloadLength();
				m_headerData->reserve(headerInfo->payloadLength);

				m_payloadLengthIndicatorSize.reset();
				m_headerData.reset();
			}
		}
	}

	// In payload body construction phase
	if (headerInfo)
	{
		ASSERT(headerInfo->payloadLength > 0);

		const size_t bytesLeft = BytesLeft();
		const size_t upperBound = (range.Size() < bytesLeft) ? range.Size() : bytesLeft;

		payloadData.insert(payloadData.end(), range.begin(), range.begin() + upperBound);
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

std::vector<byte> NetworkPayload::GenerateHeader(IdsCommand command, const std::vector<byte>& data)
{
	std::vector<byte> headerBytes;
	headerBytes.reserve(HEADER_MIN_LENGTH);

	{
		auto commandIdentifier = ToBytes<UInt16>((UInt16)command, Endianness::BIG_ENDIAN);
		headerBytes.insert(headerBytes.begin(), commandIdentifier.begin(), commandIdentifier.end());
	}

	if (IsInBound(data.size(), (size_t)3, (size_t)255))
	{
		headerBytes.push_back(static_cast<UInt8>(data.size()));
	}
	else
	{
		if (CanFit<UInt16>(data.size()))
		{
			headerBytes.push_back(0x00);
			headerBytes.reserve(HEADER_MIN_LENGTH + sizeof(UInt16));

			auto dataLength = ToBytes<UInt16>((UInt16)data.size(), Endianness::BIG_ENDIAN);
			headerBytes.insert(headerBytes.begin(), dataLength.begin(), dataLength.end());
		}

		else if (CanFit<UInt32>(data.size()))
		{
			headerBytes.push_back(0x01);
			headerBytes.reserve(HEADER_MIN_LENGTH + sizeof(UInt32));

			auto dataLength = ToBytes<UInt32>((UInt32)data.size(), Endianness::BIG_ENDIAN);
			headerBytes.insert(headerBytes.begin(), dataLength.begin(), dataLength.end());
		}

		else if (CanFit<UInt64>(data.size()))
		{
			headerBytes.push_back(0x02);
			headerBytes.reserve(HEADER_MIN_LENGTH + sizeof(UInt64));

			auto dataLength = ToBytes<UInt64>((UInt64)data.size(), Endianness::BIG_ENDIAN);
			headerBytes.insert(headerBytes.begin(), dataLength.begin(), dataLength.end());
		}
	}

	return headerBytes;
}
