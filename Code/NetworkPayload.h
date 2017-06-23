#pragma once
#include "IdsCommand.hpp"
#include "Range.hpp"
#include <array>
#include "ReadBufferSize.hpp"
#include <optional>

#define HEADER_MIN_LENGTH 3

struct NetworkMessageHeader
{
	IdsCommand command;
	UInt64 payloadLength = 0;
};

struct NetworkPayload
{
	std::optional<NetworkMessageHeader> headerInfo;
	std::vector<byte> payloadData;

public:
	NetworkPayload();
	void Insert(Range<std::array<byte, READ_BUFFER_SIZE>>& data);
	size_t BytesLeft() const;
	
private:
	void ReadHeaderFirstPart(Range<std::array<byte, READ_BUFFER_SIZE>>& data);
	void ReadHeaderPayloadLength(Range<std::array<byte, READ_BUFFER_SIZE>>& data);

	std::optional<std::vector<byte>> m_headerData;
	std::optional<UInt16> m_payloadLengthIndicatorSize;
};