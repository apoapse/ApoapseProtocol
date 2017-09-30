#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "Range.hpp"

const std::array<int, 5> testArray { 1, 2, 3, 4, 5 };

UNIT_TEST("Range:Consume:size")
{
	Range<std::array<int, 5>> testRange(testArray);
	testRange.Consume(2);

	UnitTest::Assert(testRange.size() == 3);
} UNIT_TEST_END

UNIT_TEST("Range:Consume:size:full")
{
	Range<std::array<int, 5>> testRange(testArray);
	testRange.Consume(testArray.size());

	UnitTest::Assert(testRange.size() == 0);
} UNIT_TEST_END

UNIT_TEST("Range:Consume:ReadContent")
{
	Range<std::array<int, 5>> testRange(testArray);
	testRange.Consume(3);

	UnitTest::Assert(testRange[0] == 4 && testRange.at(1) == 5);
} UNIT_TEST_END

UNIT_TEST("Range:Consume:GetCursorPosition")
{
	Range<std::array<int, 5>> testRange(testArray);
	testRange.Consume(4);

	UnitTest::Assert(testRange.GetCursorPosition() == 4);
} UNIT_TEST_END

UNIT_TEST("Range:Consume:iterators")
{
	Range<std::array<int, 5>> testRange(testArray);
	testRange.Consume(3);
	std::vector<int> testVec;
	testVec.insert(testVec.begin(), testRange.begin(), testRange.end());

	UnitTest::Assert(testVec.size() == 2 && testVec[0] == 4 && testVec[1] == 5);
} UNIT_TEST_END

UNIT_TEST("Range:Consume:data")
{
	Range<std::array<int, 5>> testRange(testArray);
	testRange.Consume(2);

	std::array<int, 3> testArr;
	memcpy(testArr.data(), testRange.data(), testRange.size() * sizeof(int));

	UnitTest::Assert(testArr[0] == 3 && testArr[1] == 4 && testArr[2] == 5, std::to_string(testArr[0]));
} UNIT_TEST_END

UNIT_TEST("Range:Consume:upperBound")
{
	Range<std::array<int, 5>> testRange(testArray, 2);

	UnitTest::Assert(testRange[0] == 1 && testRange.size() == 2);
} UNIT_TEST_END

UNIT_TEST("Range:nested_range")
{
	Range<std::array<int, 5>> testRange(testArray);
	testRange.Consume(2);

	Range<std::array<int, 5>> testRange2(testRange);

	testRange2.Consume(2);

	UnitTest::Assert(testRange2[0] == 5);
} UNIT_TEST_END


#endif	// UNIT_TESTS