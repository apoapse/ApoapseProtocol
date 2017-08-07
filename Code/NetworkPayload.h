#pragma once
#include "Commands.hpp"
#include "Range.hpp"
#include <array>
#include "ReadBufferSize.hpp"
#include <optional>

#define HEADER_MIN_LENGTH 3

struct NetworkMessageHeader
{
	CommandId command;
	UInt64 payloadLength = 0;
};

class NetworkPayload
{
public:
	static constexpr size_t payloadMaxAllowedLength = 8'000'000;	// 8 MB

	std::optional<NetworkMessageHeader> headerInfo;
	std::vector<byte> payloadData;

	void Insert(Range<std::array<byte, READ_BUFFER_SIZE>>& data);
	size_t BytesLeft() const;

	NetworkPayload() = default;

	// WARNING: Make sure to pass the data with an std::move
	NetworkPayload(CommandId command, std::vector<byte>&& data);

	static std::vector<byte> GenerateHeader(CommandId command, const std::vector<byte>& data);
	std::vector<byte> GetHeaderData() const;
	
private:
	void ReadHeaderFirstPart();
	void ReadHeaderPayloadLength();

	std::optional<std::vector<byte>> m_headerData;
	std::optional<UInt16> m_payloadLengthIndicatorSize;
	bool m_isFirstInsert{ true };
};