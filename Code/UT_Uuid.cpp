#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "Uuid.h"

UNIT_TEST("Uuid:generate_and_validate")
{
	auto uuid = Uuid::Generate();

	std::vector<byte> vec(uuid.GetInRawFormat().begin(), uuid.GetInRawFormat().end());

	UnitTest::Assert(Uuid::IsValid(vec));
} UNIT_TEST_END

UNIT_TEST("Uuid:validate:too_short")
{
	std::vector<byte> vec{0x01, 0x02, 0x03, 0x04, 0x01, 0x01, 0x01 };

	UnitTest::Assert(!Uuid::IsValid(vec));
} UNIT_TEST_END

UNIT_TEST("Uuid:validate:too_long")
{
	std::vector<byte> vec{ 0x01, 0x02, 0x03, 0x04, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01 };

	UnitTest::Assert(!Uuid::IsValid(vec));
} UNIT_TEST_END

#endif	// UNIT_TESTS