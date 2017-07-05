#pragma once
#include "Commands.hpp"
#include "Range.hpp"
#include <array>
#include "ReadBufferSize.hpp"
#include <optional>

#define HEADER_MIN_LENGTH 3

struct NetworkMessageHeader
{
	Commands command;
	UInt64 payloadLength = 0;
};

struct NetworkPayload
{
	std::optional<NetworkMessageHeader> headerInfo;
	std::vector<byte> payloadData;

	void Insert(Range<std::array<byte, READ_BUFFER_SIZE>>& data);
	size_t BytesLeft() const;

	static std::vector<byte> GenerateHeader(Commands command, const std::vector<byte>& data);
	
private:
	void ReadHeaderFirstPart();
	void ReadHeaderPayloadLength();

	std::optional<std::vector<byte>> m_headerData;
	std::optional<UInt16> m_payloadLengthIndicatorSize;
	bool m_isFirstInsert{ true };
};