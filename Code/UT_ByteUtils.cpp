#ifdef UNIT_TESTS
#include "UnitTestsManager.h"
#include "ByteUtils.hpp"

UNIT_TEST("ByteUtils:ToBytes:LittleEndian")
{
	std::array<byte, 4> bytes{ 0x74, 0x74, 0x01, 0x00 };

	UnitTest::Assert(ToBytes<int, Endianness::LITTLE_ENDIAN>(int(95348)) == bytes);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:ToBytes:BigEndian")
{
	std::array<byte, 4> bytes{ 0xff, 0xff, 0xfe, 0x50 };

	UnitTest::Assert(ToBytes<int, Endianness::BIG_ENDIAN>(int(-432)) == bytes);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:LittleEndian")
{
	std::array<byte, 4> bytes{ 0x74, 0x74, 0x01, 0x00 };
	auto result = FromBytes<int, Endianness::LITTLE_ENDIAN>(bytes);

	UnitTest::Assert(result == 95348);
} UNIT_TEST_END

UNIT_TEST("ByteUtils:FromBytes:BigEndian")
{
	std::array<byte, 4> bytes{ 0xff, 0xff, 0xfe, 0x50 };
	auto result = FromBytes<int, Endianness::BIG_ENDIAN>(bytes);

	UnitTest::Assert(result == -432);
} UNIT_TEST_END

#endif