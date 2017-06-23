#include "UnitTestsManager.h"
#include "NetworkPayload.h"
// 
// UNIT_TEST("NetworkPayload:ReadHeader:16bitLength")
// {
// 	std::vector<byte> headerData{ 0x00, 0x01, 0x00, 0x00, 0x03 };
// 	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
// 	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());
// 
// 	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr);
// 
// 	auto payloadInfo = NetworkPayload(range);
// 
// 	UnitTest::Assert(payloadInfo.headerInfo.command == IdsCommand::CONNECT && payloadInfo.headerInfo.payloadLength == 3);
// } UNIT_TEST_END
// 
// UNIT_TEST("NetworkPayload:ReadHeader:32bitLength")
// {
// 	std::vector<byte> headerData{ 0x00, 0x01, 0x01, 0x00, 0x0f, 0x0c, 0x86 };
// 	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
// 	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());
// 
// 	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr);
// 
// 	auto payloadInfo = NetworkPayload(range);
// 
// 	UnitTest::Assert(payloadInfo.headerInfo.command == IdsCommand::CONNECT && payloadInfo.headerInfo.payloadLength == 986246);
// } UNIT_TEST_END
// 
// UNIT_TEST("NetworkPayload:ReadHeader:64bitLength")
// {
// 	std::vector<byte> headerData{ 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x2a, 0x05, 0xf2, 0x00 };
// 	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
// 	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());
// 
// 	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr);
// 
// 	auto payloadInfo = NetworkPayload(range);
// 
// 	UnitTest::Assert(payloadInfo.headerInfo.command == IdsCommand::ERROR_MSG && payloadInfo.headerInfo.payloadLength == 5000000000);
// } UNIT_TEST_END
// 
// UNIT_TEST("NetworkPayload:ReadHeader:8bitLength")
// {
// 	std::vector<byte> headerData{ 0x00, 0x01, 0x56 };
// 	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
// 	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());
// 
// 	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr);
// 
// 	auto payloadInfo = NetworkPayload(range);
// 
// 	UnitTest::Assert(payloadInfo.headerInfo.command == IdsCommand::CONNECT && payloadInfo.headerInfo.payloadLength == 86);
// } UNIT_TEST_END
// 
// UNIT_TEST("NetworkPayload:ReadHeader:8bitLength:2")
// {
// 	std::vector<byte> headerData{ 0x00, 0x00, 0x03 };
// 	std::array<byte, READ_BUFFER_SIZE> headerDataArr;
// 	std::copy(headerData.begin(), headerData.end(), headerDataArr.begin());
// 
// 	Range<std::array<byte, READ_BUFFER_SIZE>> range(headerDataArr);
// 
// 	auto payloadInfo = NetworkPayload(range);
// 
// 	UnitTest::Assert(payloadInfo.headerInfo.command == IdsCommand::ERROR_MSG && payloadInfo.headerInfo.payloadLength == 3);
// } UNIT_TEST_END