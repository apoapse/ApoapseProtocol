#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "CommandsManager.h"
#include "Command.h"
#include "MessagePack.hpp"
#include "Uuid.h"
#include "NetworkPayload.h"

UNIT_TEST("CommandsManager:CommandExist")
{
	UnitTest::Assert(CommandsManager::GetInstance().CommandExist(CommandId::unit_test_cmd));
} UNIT_TEST_END

UNIT_TEST("CommandsManager:CreateCommand_GetInfo")
{
	auto cmd = CommandsManager::GetInstance().CreateCommand(CommandId::unit_test_cmd);

	UnitTest::Assert(cmd->GetInfo().command == CommandId::unit_test_cmd);
} UNIT_TEST_END

UNIT_TEST("Command:Parse:Validate:valid")
{
	MessagePackSerializer serializer;
	serializer.UnorderedAppend("mendatory_field_uuid_validator", Uuid::Generate().GetAsByteVector());
	serializer.UnorderedFieldsCompile();
	auto msgpkData = serializer.GetMessagePackBytes();

	std::vector<byte> data(msgpkData.size() + NetworkPayload::headerLength);
	std::copy(msgpkData.begin(), msgpkData.end(), data.begin() + NetworkPayload::headerLength);

	auto payload = std::make_shared<NetworkPayload>(CommandId::unit_test_cmd, std::move(data));
	auto cmd = CommandsManager::GetInstance().CreateCommand(CommandId::unit_test_cmd);
	cmd->Parse(payload);

	UnitTest::Assert(cmd->IsValid());
} UNIT_TEST_END

UNIT_TEST("Command:Parse:Validate:invalid:validator_fail")
{
	MessagePackSerializer serializer;
	serializer.UnorderedAppend("mendatory_field_uuid_validator", "test"s);
	serializer.UnorderedFieldsCompile();
	auto msgpkData = serializer.GetMessagePackBytes();

	std::vector<byte> data(msgpkData.size() + NetworkPayload::headerLength);
	std::copy(msgpkData.begin(), msgpkData.end(), data.begin() + NetworkPayload::headerLength);

	auto payload = std::make_shared<NetworkPayload>(CommandId::unit_test_cmd, std::move(data));
	auto cmd = CommandsManager::GetInstance().CreateCommand(CommandId::unit_test_cmd);
	cmd->Parse(payload);

	UnitTest::Assert(!cmd->IsValid());
} UNIT_TEST_END

UNIT_TEST("Command:Parse:Validate:invalid:mendatory_value_missing")
{
	MessagePackSerializer serializer;
	serializer.UnorderedAppend<UInt16>("optional_field", 15);
	serializer.UnorderedFieldsCompile();
	auto msgpkData = serializer.GetMessagePackBytes();

	std::vector<byte> data(msgpkData.size() + NetworkPayload::headerLength);
	std::copy(msgpkData.begin(), msgpkData.end(), data.begin() + NetworkPayload::headerLength);

	auto payload = std::make_shared<NetworkPayload>(CommandId::unit_test_cmd, std::move(data));
	auto cmd = CommandsManager::GetInstance().CreateCommand(CommandId::unit_test_cmd);
	cmd->Parse(payload);

	UnitTest::Assert(!cmd->IsValid());
} UNIT_TEST_END

UNIT_TEST("Command:Parse:Validate:invalid:wrong_optional_value")
{
	MessagePackSerializer serializer;
	serializer.UnorderedAppend<std::string>("optional_field", "should be a UInt16");
	serializer.UnorderedAppend("mendatory_field_uuid_validator", Uuid::Generate().GetAsByteVector());
	serializer.UnorderedFieldsCompile();
	auto msgpkData = serializer.GetMessagePackBytes();

	std::vector<byte> data(msgpkData.size() + NetworkPayload::headerLength);
	std::copy(msgpkData.begin(), msgpkData.end(), data.begin() + NetworkPayload::headerLength);

	auto payload = std::make_shared<NetworkPayload>(CommandId::unit_test_cmd, std::move(data));
	auto cmd = CommandsManager::GetInstance().CreateCommand(CommandId::unit_test_cmd);
	cmd->Parse(payload);

	UnitTest::Assert(!cmd->IsValid());
} UNIT_TEST_END

#endif	// UNIT_TESTS