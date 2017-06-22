#pragma once
#include "IdsCommand.hpp"
#include "Range.hpp"
#include <array>
#include <vector>
#include "ReadBufferSize.hpp"

struct NetworkMessageHeader
{
	IdsCommand command;
	UInt64 payloadLength = 0;
};

struct NetworkPayload
{
	NetworkMessageHeader headerInfo;
	std::vector<byte> payloadData;

	NetworkPayload(Range<std::array<byte, READ_BUFFER_SIZE>>& workingData);
	size_t BytesLeft() const;

private:
	static NetworkMessageHeader ReadHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& data);
};