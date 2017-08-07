#include "stdafx.h"

#include "UnitTestsManager.h"
#include "NetworkPayload.h"

UNIT_TEST("NetworkPayload:ReadHeader:16bitLength")
{
	std::vector<byte> headerData{ 0x00, 0x02, 0x00, 0x00, 0x03 };
	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());

	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr, headerData.size());

	auto payload = NetworkPayload();
	payload.Insert(range);

	UnitTest::Assert(payload.headerInfo->command == CommandId::connect && payload.headerInfo->payloadLength == 3);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:ReadHeader:32bitLength")
{
	std::vector<byte> headerData{ 0x00, 0x02, 0x01, 0x00, 0x0f, 0x0c, 0x86 };
	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());

	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr, headerData.size());

	auto payload = NetworkPayload();
	payload.Insert(range);

	UnitTest::Assert(payload.headerInfo->command == CommandId::connect && payload.headerInfo->payloadLength == 986246);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:ReadHeader:64bitLength")
{
	std::vector<byte> headerData{ 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x01, 0x2a, 0x05, 0xf2, 0x00 };
	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());

	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr, headerData.size());

	auto payload = NetworkPayload();
	payload.Insert(range);

	UnitTest::Assert(payload.headerInfo->command == CommandId::error_msg && payload.headerInfo->payloadLength == 5000000000);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:ReadHeader:8bitLength")
{
	std::vector<byte> headerData{ 0x00, 0x02, 0x56 };
	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());

	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr, headerData.size());

	auto payload = NetworkPayload();
	payload.Insert(range);

	UnitTest::Assert(payload.headerInfo->command == CommandId::connect && payload.headerInfo->payloadLength == 86);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:ReadHeader:8bitLength:2")
{
	std::vector<byte> headerData{ 0x00, 0x01, 0x03 };
	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());

	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr, headerData.size());

	auto payload = NetworkPayload();
	payload.Insert(range);

	UnitTest::Assert(payload.headerInfo->command == CommandId::error_msg && payload.headerInfo->payloadLength == 3);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:InsertRawData")
{
	std::vector<byte> headerData{ 0x00, 0x02, 0x03, 0x01, 0x02, 0x03, 0xff };
	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());

	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr, headerData.size());

	auto payload = NetworkPayload();
	payload.Insert(range);
	UnitTest::Assert(payload.headerInfo->command == CommandId::connect && payload.headerInfo->payloadLength == 3 && payload.payloadData.size() == 3 && range.size() == 1);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:GenerateHeader")
{
	std::vector<byte> data{ 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0xff };
	auto header = NetworkPayload::GenerateHeader(CommandId::connect, data);

	UnitTest::Assert(header.size() == 3);
} UNIT_TEST_END

UNIT_TEST("NetworkPayload:GenerateHeader2")
{
	std::vector<byte> data{ 0x01, 0x02, 0x03, 0x01, 0x02, 0x03, 0xff };
	auto header = NetworkPayload::GenerateHeader(CommandId::connect, data);
	std::array<byte, READ_BUFFER_SIZE> headerDataArr;

	std::copy(header.begin(), header.end(), headerDataArr.begin());
	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr, header.size());

	auto payload = NetworkPayload();
	payload.Insert(range);

	UnitTest::Assert(payload.headerInfo->command == CommandId::connect && payload.headerInfo->payloadLength == 7);
} UNIT_TEST_END