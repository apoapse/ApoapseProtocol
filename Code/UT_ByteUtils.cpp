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

#include "stdafx.h"

#include "UnitTest.hpp"
#include "ByteUtils.hpp"

UNIT_TEST("ByteUtils:ToBytes:LittleEndian")
{
	std::array<byte, 4> bytes{ 0x74, 0x74, 0x01, 0x00 };

	CHECK(ToBytes<int>(int(95348), Endianness::AP_LITTLE_ENDIAN) == bytes);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:ToBytes:BigEndian")
{
	std::array<byte, 4> bytes{ 0xff, 0xff, 0xfe, 0x50 };

	CHECK(ToBytes<int>(int(-432), Endianness::AP_BIG_ENDIAN) == bytes);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:LittleEndian")
{
	std::array<byte, 4> bytes{ 0x74, 0x74, 0x01, 0x00 };
	auto result = FromBytes<int>(bytes, Endianness::AP_LITTLE_ENDIAN);

	CHECK(result == 95348);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:BigEndian")
{
	std::array<byte, 4> bytes{ 0xff, 0xff, 0xfe, 0x50 };
	auto result = FromBytes<int>(bytes, Endianness::AP_BIG_ENDIAN);

	CHECK(result == -432);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:Range:LittleEndian")
{
	std::vector<byte> bytes{ 0x74, 0x74, 0x01, 0x00 };
	Range<std::vector<byte>> range(bytes);

	auto result = FromBytes<int>(range, Endianness::AP_LITTLE_ENDIAN);

	CHECK(result == 95348);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:Range:BigEndian")
{
	std::vector<byte> bytes{ 0xff, 0xff, 0xfe, 0x50 };
	Range<std::vector<byte>> range(bytes);

	auto result = FromBytes<int>(range, Endianness::AP_BIG_ENDIAN);

	CHECK(result == -432);
} UNIT_TEST_END