// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once
#include "Range.hpp"
#include <array>
#include "ReadBufferSize.hpp"
#include <optional>

struct NetworkMessageHeader
{
	std::string cmdShortName;
	UInt32 payloadLength = 0;
};

class NetworkPayload
{
	std::vector<byte> m_rawPayloadData;

public:
	static constexpr size_t payloadMaxAllowedLength = 8'000'000;	// 8 MB
	static constexpr UInt8 headerLength = 6;						// UInt16 for the command id + UInt32 for the content length
	std::optional<NetworkMessageHeader> headerInfo;

	NetworkPayload() = default;
	NetworkPayload(const std::string& cmdShortName, std::vector<byte>&& data);	// WARNING: Make sure to pass the data with an std::move

	void Insert(Range<std::array<byte, READ_BUFFER_SIZE>>& data, size_t length);
	Range<std::vector<byte>> GetPayloadContent() const;
	const std::vector<byte>& GetRawData() const;
	UInt32 BytesLeft() const;

//private:	// Put public for unit testing
	void ReadHeader();

private:
	static void WriteHeader(const std::string& cmdShortName, std::vector<byte>& data);
};