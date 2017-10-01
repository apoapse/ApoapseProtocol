#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "NetworkPayload.h"

const std::vector<byte> srcData{ 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x01, 0x02 };

UNIT_TEST("NetworkPayload:Insert:ReadHeader")
{
	std::array<byte, READ_BUFFER_SIZE> testArr;
	std::copy(srcData.begin(), srcData.end(), testArr.begin());
	Range<std::array<byte, READ_BUFFER_SIZE>> range(testArr);

	NetworkPayload payload;
	payload.Insert(range, 4);

	payload.Insert(range, 4);

	UnitTest::Assert(payload.headerInfo->payloadLength == 2 && payload.headerInfo->command == CommandId::connect, std::to_string(payload.headerInfo->payloadLength));
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:Insert:BytesLeft")
{
	std::array<byte, READ_BUFFER_SIZE> testArr;
	std::copy(srcData.begin(), srcData.end(), testArr.begin());
	Range<std::array<byte, READ_BUFFER_SIZE>> range(testArr);

	NetworkPayload payload;

	if (payload.BytesLeft() == 0)
		UnitTest::Fail("#1");

	payload.Insert(range, 4);

	if (payload.BytesLeft() != NetworkPayload::headerLength - 4)
		UnitTest::Fail("#2");

	payload.Insert(range, 4);

	UnitTest::Assert(payload.BytesLeft() == 0, std::to_string(payload.BytesLeft()));
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
	UnitTest::Assert(payloadContent[0] == 0x01 && payloadContent[1] == 0x02);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:WriteHeader")
{
	std::vector<byte> testVec(8);
	testVec[6] = 0x01;
	testVec[7] = 0x02;

	NetworkPayload payload(CommandId::error_msg, std::move(testVec));
	payload.ReadHeader();

	UnitTest::Assert(payload.headerInfo->payloadLength == 2 && payload.headerInfo->command == CommandId::error_msg);
} UNIT_TEST_END

#endif	// UNIT_TESTS