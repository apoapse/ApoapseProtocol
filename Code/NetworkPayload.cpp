#include "stdafx.h"
#include "NetworkPayload.h"
#include "ByteUtils.hpp"
#include "Diagnostic.hpp"
#include "Maths.hpp"

NetworkPayload::NetworkPayload(const std::string& cmdShortName, std::vector<byte>&& data)
{
	m_rawPayloadData = std::move(data);
	WriteHeader(cmdShortName, m_rawPayloadData);

	headerInfo = NetworkMessageHeader();
	headerInfo->cmdShortName = cmdShortName;
	headerInfo->payloadLength = (static_cast<UInt32>(m_rawPayloadData.size()) - headerLength);
}

void NetworkPayload::Insert(Range<std::array<byte, READ_BUFFER_SIZE>>& range, size_t length)
{
	const size_t currentPayloadLength = std::min(std::min((length), static_cast<size_t>(BytesLeft())), range.size());

	m_rawPayloadData.insert(m_rawPayloadData.end(), range.begin(), range.begin() + currentPayloadLength);
	range.Consume(currentPayloadLength);

	if (!headerInfo.has_value() && m_rawPayloadData.size() >= headerLength)
	{
		ReadHeader();

		const size_t bytesLeft = BytesLeft();
		if (range.size() > 0 && bytesLeft > 0)
			Insert(range, std::min(range.size(), bytesLeft));
	}
}

Range<std::vector<byte>> NetworkPayload::GetPayloadContent() const
{
	ASSERT_MSG(BytesLeft() == 0, "Trying to access to the payload data while the payload is not complete yet");

	Range<std::vector<byte>> range(m_rawPayloadData);
	range.Consume(headerLength);

	return range;
}

const std::vector<byte>& NetworkPayload::GetRawData() const
{
	return m_rawPayloadData;
}

UInt32 NetworkPayload::BytesLeft() const
{
	if (headerInfo.has_value())
		return static_cast<UInt32>(headerInfo->payloadLength - (m_rawPayloadData.size() - headerLength));
	else
		return std::max(static_cast<UInt32>(headerLength - m_rawPayloadData.size()), (UInt32)1);	// Make sure the size can never be to 0 in this particular case
}

void NetworkPayload::ReadHeader()
{
	ASSERT(m_rawPayloadData.size() >= headerLength);

	Range<std::vector<byte>> range(m_rawPayloadData);

	headerInfo = NetworkMessageHeader();

	{
		headerInfo->cmdShortName = std::string(range.begin(), range.begin() + 2);
		range.Consume(2);
	}

	{
		headerInfo->payloadLength = FromBytes<UInt32>(range, Endianness::BIG_ENDIAN);

		if (headerInfo->payloadLength > payloadMaxAllowedLength)
			throw std::length_error("");
	}
	//range.Consume(sizeof(UInt32));	// Not needed since it's the last operatation on the temporary Range
}

void NetworkPayload::WriteHeader(const std::string& cmdShortName, std::vector<byte>& data)
{
	ASSERT(data.size() >= headerLength);
	ASSERT_MSG(CanFit<UInt32>(data.size()) , "The vector provided is to big for its size to fit into a UInt32");

	std::copy(cmdShortName.begin(), cmdShortName.end(), data.begin());

	{
		const UInt32 pyaloadContentSize = (static_cast<UInt32>(data.size()) - headerLength);
		const auto payloadLength = ToBytes<UInt32>(static_cast<UInt32>(pyaloadContentSize), Endianness::BIG_ENDIAN);
		std::copy(payloadLength.begin(), payloadLength.end(), data.begin() + cmdShortName.size());
	}
}
