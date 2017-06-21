#pragma once
#include "CommandNames.hpp"
#include "Range.hpp"
#include <array>
#include "ReadBufferSize.hpp"

struct NetworkMessageHeader
{
	CommandNames commandName;
	UInt64 payloadLength;
};

NetworkMessageHeader ReadHeader(Range<std::array<byte, READ_BUFFER_SIZE>>& data);