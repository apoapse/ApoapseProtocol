#include "stdafx.h"

#include "UnitTestsManager.h"
#include "StringExtensions.h"

UNIT_TEST("StringExtensions:erase_left")
{
	std::string str = "test";
	StringExtensions::erase_left(str, 's');

	CHECK(str == "st");
} UNIT_TEST_END

UNIT_TEST("StringExtensions:erase_right")
{
	std::string str = "abcdefg";
	StringExtensions::erase_right(str, 'd');

	CHECK(str == "abcd");
} UNIT_TEST_END

UNIT_TEST("StringExtensions:erase_all")
{
	std::string str = "tests";
	StringExtensions::erase_all(str, 's');

	CHECK(str == "tet");
} UNIT_TEST_END

UNIT_TEST("StringExtensions:trim_left")
{
	std::string str = " test \n";
	StringExtensions::trim_left(str);

	CHECK(str == "test \n");
} UNIT_TEST_END

UNIT_TEST("StringExtensions:trim_right")
{
	std::string str = " test \n";
	StringExtensions::trim_right(str);

	CHECK(str == " test");
} UNIT_TEST_END

UNIT_TEST("StringExtensions:split")
{
	std::string str = "StringExtensions/split";
	std::vector<std::string> resultValues;

	StringExtensions::split(str, resultValues, std::string("/"));

	CHECK(resultValues.size() == 2);
	CHECK(resultValues.at(0) == "StringExtensions");
	CHECK(resultValues.at(1) == "split");
} UNIT_TEST_END

UNIT_TEST("StringExtensions:get_last_chars")
{
	std::string str = "StringExtensions";

	CHECK(StringExtensions::get_last_chars(str, 3) == "ons");
} UNIT_TEST_END

UNIT_TEST("StringExtensions:contains")
{
	std::string str = "String:Extensions";

	CHECK(StringExtensions::contains(str, ':'));
} UNIT_TEST_END

UNIT_TEST("StringExtensions:IsOnlyAlphanumeric_alphanumeric")
{
	std::string str = "alpha16897fzeaFAgfHGL";

	CHECK(StringExtensions::IsOnlyAlphanumeric(str));
} UNIT_TEST_END

UNIT_TEST("StringExtensions:IsOnlyAlphanumeric_specialChars")
{
	std::string str = "alph_a16897fzeaFAgfHGL";

	CHECK(!StringExtensions::IsOnlyAlphanumeric(str));
} UNIT_TEST_END
