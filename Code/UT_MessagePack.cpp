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

	CHECK(deserializer.GetValue<std::string>("string1") == "string_value1");
	CHECK(deserializer.GetValue<Int16>("int1") == -26);
	CHECK(deserializer.GetArray<int>("test_array").at(1) == 7);
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered")
{
	MessagePackSerializer ser;
	ser.UnorderedAppend("test", "coucou"s);
	ser.UnorderedAppend("testb", true);
	ser.UnorderedAppendArray("loli.arr_test", std::vector<std::string>{"hey", "lul"});

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	CHECK(deserializer.GetValue<std::string>("test") == "coucou");
	CHECK(deserializer.GetValue<bool>("testb") == true);
	CHECK(deserializer.GetArray<std::string>("loli.arr_test").at(1) == "lul");
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered_2")
{
	std::vector<byte> data(2);

	MessagePackSerializer ser;
	ser.UnorderedAppend("bytes", data);
	ser.UnorderedAppend<bool>("testb", true);

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	auto test = ser.GetMessagePackBytes();

	CHECK(deserializer.GetValue<std::vector<byte>>("bytes").size() == 2);
	CHECK(deserializer.GetValue<bool>("testb"));
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:no_root:ordered")
{
	MessagePackSerializer serializer;
	std::vector<int> arrayData{ 6, 7, 8, -625 };

	serializer.Group("",
	{
		MSGPK_ORDERED_APPEND_ARRAY(serializer, int, "root.test_array", arrayData),
		MSGPK_ORDERED_APPEND(serializer, "root.int1", -26),
	});

	MessagePackDeserializer deserializer(serializer.GetMessagePackBytes());
	auto test = serializer.GetMessagePackBytes();

	CHECK(deserializer.GetValue<Int16>("root.int1") == -26);
	CHECK(deserializer.GetArray<int>("root.test_array").at(1) == 7);
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:no_root:unordered")
{
	MessagePackSerializer serializer;
	std::vector<int> arrayData{ 6, 7, 8, -625 };

	serializer.UnorderedAppendArray("root.test_array", arrayData);
	serializer.UnorderedAppend("root.int1", -26);

	MessagePackDeserializer deserializer(serializer.GetMessagePackBytes());

	CHECK(deserializer.GetValue<Int16>("root.int1") == -26);
	CHECK(deserializer.GetArray<int>("root.test_array").at(1) == 7);
	
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:unordered:std_array")
{
	std::array<byte, 2> arrayTest{ 0xf3, 0x08 };

	MessagePackSerializer ser;
	ser.UnorderedAppend("arr", arrayTest);

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	const auto val = deserializer.GetValue<std::vector<byte>>("arr");
	CHECK(val[0] == 0xf3);
	CHECK(val[1] == 0x08);

} UNIT_TEST_END

UNIT_TEST("MessagePack:deserialize:optional")
{
	MessagePackSerializer ser;
	ser.UnorderedAppend("test", "coucou"s);

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	CHECK(deserializer.GetValueOptional<std::string>("test").has_value());
	CHECK(!deserializer.GetValueOptional<std::string>("test.noes_not_exist"));
} UNIT_TEST_END

UNIT_TEST("MessagePack:serialize:ordered:bytes")
{
	MessagePackSerializer serializer;

	serializer.Group("",
	{
		MSGPK_ORDERED_APPEND(serializer, "related_item", Uuid::Generate().GetBytes()),
		MSGPK_ORDERED_APPEND(serializer, "test_int", 32)
	});

	MessagePackDeserializer deserializer(serializer.GetMessagePackBytes());

	CHECK(Uuid::IsValid(deserializer.GetValue<std::vector<byte>>("related_item")));
	CHECK(deserializer.GetValue<int>("test_int") == 32);
	
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

	CHECK(deserializer.GetValue<MessagePackDeserializer>("nested").GetValue<std::string>("str") == "test string");
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

		REQUIRE(!finalBytes.empty());
	}

	MessagePackDeserializer deserializer(finalBytes);
	auto deserializersArray = deserializer.GetArray<MessagePackDeserializer>("nested");

	CHECK(deserializersArray[0].GetValue<std::string>("test_str") == "test_str_0");
	CHECK(deserializersArray[1].GetValue<std::string>("test_str") == "test_str_1");
	
} UNIT_TEST_END

UNIT_TEST("MessagePack:ToSerializer")
{
	MessagePackSerializer ser;
	{
		MessagePackSerializer nestedSer;
		nestedSer.UnorderedAppend("str", "test string"s);

		ser.UnorderedAppend("nested", nestedSer);
		ser.UnorderedAppend("simple", 58);
	}

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());
	auto serializerFinal = deserializer.ToSerializer();
	MessagePackDeserializer deserializerFinal(serializerFinal.GetMessagePackBytes());

	CHECK(deserializerFinal.GetValue<int>("simple") == 58);
	CHECK(deserializerFinal.GetValue<MessagePackDeserializer>("nested").GetValue<std::string>("str") == "test string");
	
} UNIT_TEST_END

#endif	// UNIT_TESTS