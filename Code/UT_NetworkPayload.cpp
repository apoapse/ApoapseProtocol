#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTest.hpp"
#include "NetworkPayload.h"

const std::vector<byte> srcData{ 0x65, 0x72, 0x00, 0x00, 0x00, 0x02, 0x01, 0x02 };

UNIT_TEST("NetworkPayload:Insert:ReadHeader")
{
	std::array<byte, READ_BUFFER_SIZE> testArr;
	std::copy(srcData.begin(), srcData.end(), testArr.begin());
	Range<std::array<byte, READ_BUFFER_SIZE>> range(testArr);

	NetworkPayload payload;
	payload.Insert(range, 4);

	payload.Insert(range, 4);

	CHECK(payload.headerInfo->payloadLength == 2);
	CHECK(payload.headerInfo->cmdShortName == "er");
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:Insert:BytesLeft")
{
	std::array<byte, READ_BUFFER_SIZE> testArr;
	std::copy(srcData.begin(), srcData.end(), testArr.begin());
	Range<std::array<byte, READ_BUFFER_SIZE>> range(testArr);

	NetworkPayload payload;

	REQUIRE(payload.BytesLeft() != 0);
	payload.Insert(range, 4);

	REQUIRE(!(payload.BytesLeft() != NetworkPayload::headerLength - 4));
	payload.Insert(range, 4);

	CHECK(payload.BytesLeft() == 0);
	
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:Insert:GetPayloadContent")
{
	std::array<byte, READ_BUFFER_SIZE> testArr;
	std::copy(srcData.begin(), srcData.end(), testArr.begin());
	Range<std::array<byte, READ_BUFFER_SIZE>> range(testArr);

	NetworkPayload payload;
	payload.Insert(range, 4);

	payload.Insert(range, 4);

	auto payloadContent = payload.GetPayloadContent();

	CHECK(payloadContent[0] == 0x01);
	CHECK(payloadContent[1] == 0x02);
	
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:WriteHeader")
{
	std::vector<byte> testVec(8);
	testVec[6] = 0x01;
	testVec[7] = 0x02;

	NetworkPayload payload("er", std::move(testVec));
	payload.ReadHeader();

	CHECK(payload.headerInfo->payloadLength == 2);
	CHECK(payload.headerInfo->cmdShortName == "er");
	
} UNIT_TEST_END

#endif	// UNIT_TESTS