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

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	UnitTest::Assert(deserializer.GetValue<std::string>("test") == "coucou" && deserializer.GetValue<bool>("testb") == true && deserializer.GetArray<std::string>("loli.arr_test").at(1) == "lul");
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered_2")
{
	std::vector<byte> data(2);

	MessagePackSerializer ser;
	ser.UnorderedAppend("bytes", data);
	ser.UnorderedAppend<bool>("testb", true);

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	auto test = ser.GetMessagePackBytes();
	auto val = deserializer.GetValue<bool>("testb");
	UnitTest::Assert(deserializer.GetValue<std::vector<byte>>("bytes").size() == 2 && deserializer.GetValue<bool>("testb"));
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered:std_array")
{
	std::array<byte, 2> arrayTest{ 0xf3, 0x08 };

	MessagePackSerializer ser;
	ser.UnorderedAppend("arr", arrayTest);

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	const auto val = deserializer.GetValue<std::vector<byte>>("arr");
	UnitTest::Assert(val[0] == 0xf3 && val[1] == 0x08);
} UNIT_TEST_END

UNIT_TEST("MessagePack:deserialize:optional")
{
	MessagePackSerializer ser;
	ser.UnorderedAppend("test", "coucou"s);

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

UNIT_TEST("MessagePack:serialize:nested_serializer")
{
	MessagePackSerializer ser;
	{
		MessagePackSerializer nestedSer;
		nestedSer.UnorderedAppend("str", "test string"s);

		ser.UnorderedAppend("nested", nestedSer);
	}

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	UnitTest::Assert(deserializer.GetValue<MessagePackDeserializer>("nested").GetValue<std::string>("str") == "test string");
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:nested_serializer:array")
{
	std::vector<byte> finalBytes;
	{
		MessagePackSerializer ser;
		std::vector<MessagePackSerializer> serializerArray;

		for (size_t i = 0; i < 2; i++)
		{
			MessagePackSerializer serializer;
			std::string val = "test_str_" + std::to_string(i);
			serializer.UnorderedAppend("test_str", val);

			serializerArray.push_back(serializer);
		}
		ser.UnorderedAppendArray("nested", serializerArray);

		finalBytes = ser.GetMessagePackBytes();

		if (finalBytes.empty())
			UnitTest::Fail();
	}

	MessagePackDeserializer deserializer(finalBytes);
	auto deserializersArray = deserializer.GetArray<MessagePackDeserializer>("nested");

	UnitTest::Assert(deserializersArray[0].GetValue<std::string>("test_str") == "test_str_0" && deserializersArray[1].GetValue<std::string>("test_str") == "test_str_1");
} UNIT_TEST_END


#endif	// UNIT_TESTS