#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "ProtectedByteContainer.h"

UNIT_TEST("ProtectedByteContainer:Read")
{
	std::vector<byte> input;
	for (int i = 128 - 1; i >= 0; i--)
	{
		input.push_back((byte)i);
	}

	ProtectedByteContainer key(input);
	UnitTest::Assert(key.Read() == input);
} UNIT_TEST_END

#endif	// UNIT_TESTS