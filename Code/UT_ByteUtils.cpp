#include "stdafx.h"

#include "UnitTestsManager.h"
#include "ByteUtils.hpp"

UNIT_TEST("ByteUtils:ToBytes:LittleEndian")
{
	std::array<byte, 4> bytes{ 0x74, 0x74, 0x01, 0x00 };

	UnitTest::Assert(ToBytes<int>(int(95348), Endianness::LITTLE_ENDIAN) == bytes);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:ToBytes:BigEndian")
{
	std::array<byte, 4> bytes{ 0xff, 0xff, 0xfe, 0x50 };

	UnitTest::Assert(ToBytes<int>(int(-432), Endianness::BIG_ENDIAN) == bytes);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:LittleEndian")
{
	std::array<byte, 4> bytes{ 0x74, 0x74, 0x01, 0x00 };
	auto result = FromBytes<int>(bytes, Endianness::LITTLE_ENDIAN);

	UnitTest::Assert(result == 95348);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:BigEndian")
{
	std::array<byte, 4> bytes{ 0xff, 0xff, 0xfe, 0x50 };
	auto result = FromBytes<int>(bytes, Endianness::BIG_ENDIAN);

	UnitTest::Assert(result == -432);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:Range:LittleEndian")
{
	std::vector<byte> bytes{ 0x74, 0x74, 0x01, 0x00 };
	Range<std::vector<byte>> range(bytes);

	auto result = FromBytes<int>(range, Endianness::LITTLE_ENDIAN);

	UnitTest::Assert(result == 95348);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:Range:BigEndian")
{
	std::vector<byte> bytes{ 0xff, 0xff, 0xfe, 0x50 };
	Range<std::vector<byte>> range(bytes);

	auto result = FromBytes<int>(range, Endianness::BIG_ENDIAN);

	UnitTest::Assert(result == -432);
} UNIT_TEST_END