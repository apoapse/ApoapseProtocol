#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "MessagePack.hpp"
#include "Uuid.h"

UNIT_TEST("MessagePack:serialize:ordered")
{
	MessagePackSerializer serializer;
	std::vector<int> arrayData{ 6, 7, 8, -625 };

	serializer.Group("",
	{
		MSGPK_ORDERED_APPEND(serializer, "string1", "string_value1"s),
		MSGPK_ORDERED_APPEND(serializer, "int1", -26),
		MSGPK_ORDERED_APPEND_ARRAY(serializer, int, "test_array", arrayData),
	});

	MessagePackDeserializer deserializer(serializer.GetMessagePackBytes());

	UnitTest::Assert(deserializer.GetValue<std::string>("string1") == "string_value1" && deserializer.GetValue<Int16>("int1") == -26 && deserializer.GetArray<int>("test_array").at(1) == 7);
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered")
{
	MessagePackSerializer ser;
	ser.UnorderedAppend("test", "coucou"s);
	ser.UnorderedAppend("testb", true);
	ser.UnorderedAppendArray("loli.arr_test", std::vector<std::string>{"hey", "lul"});

	ser.UnorderedFieldsCompile();

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	UnitTest::Assert(deserializer.GetValue<std::string>("test") == "coucou" && deserializer.GetValue<bool>("testb") == true && deserializer.GetArray<std::string>("loli.arr_test").at(1) == "lul");
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered_2")
{
	std::vector<byte> data(2);

	MessagePackSerializer ser;
	ser.UnorderedAppend("bytes", data);
	ser.UnorderedAppend<bool>("testb", true);
	ser.UnorderedFieldsCompile();

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	auto test = ser.GetMessagePackBytes();
	auto val = deserializer.GetValue<bool>("testb");
	UnitTest::Assert(deserializer.GetValue<std::vector<byte>>("bytes").size() == 2 && deserializer.GetValue<bool>("testb"));
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered_3")
{
	MessagePackSerializer ser;
	ser.UnorderedAppend("test", "coucou"s);
	ser.UnorderedAppend("testint", 16);

	ser.UnorderedFieldsCompile();

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	UnitTest::Assert(deserializer.GetValue<std::string>("test") == "coucou" && deserializer.GetValue<int>("testint") == 16);
} UNIT_TEST_END

UNIT_TEST("MessagePack:deserialize:optional")
{
	MessagePackSerializer ser;
	ser.UnorderedAppend("test", "coucou"s);
	ser.UnorderedFieldsCompile();

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	UnitTest::Assert(deserializer.GetValueOptional<std::string>("test") && !deserializer.GetValueOptional<std::string>("test.noes_not_exist"));

} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:ordered:bytes")
{
	MessagePackSerializer serializer;

	serializer.Group("",
	{
		MSGPK_ORDERED_APPEND(serializer, "related_item", Uuid::Generate().GetAsByteVector()),
		MSGPK_ORDERED_APPEND(serializer, "test_int", 32)
	});

	MessagePackDeserializer deserializer(serializer.GetMessagePackBytes());

	UnitTest::Assert(Uuid::IsValid(deserializer.GetValue<std::vector<byte>>("related_item")) && deserializer.GetValue<int>("test_int") == 32);
} UNIT_TEST_END

#endif	// UNIT_TESTS