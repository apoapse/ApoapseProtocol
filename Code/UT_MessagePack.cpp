#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "MessagePack.hpp"

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

UNIT_TEST("MessagePack:deserialize:optional")
{
	MessagePackSerializer ser;
	ser.UnorderedAppend("test", "coucou"s);
	ser.UnorderedFieldsCompile();

	MessagePackDeserializer deserializer(ser.GetMessagePackBytes());

	UnitTest::Assert(deserializer.GetValueOptional<std::string>("test") && !deserializer.GetValueOptional<std::string>("test.noes_not_exist"));

} UNIT_TEST_END

#endif	// UNIT_TESTS