#ifdef UNIT_TESTS

#include "UnitTestsManager.h"
#include "DateTimeUtils.h"

using namespace DateTimeUtils;

UNIT_TEST("DateTimeUtils:UTCDateTime:Validate")
{
	UnitTest::Assert(UTCDateTime::ValidateFormat("2007-01-05T14:30:41Z"));
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:Validate:WrongValue")
{
	UnitTest::Assert(!UTCDateTime::ValidateFormat("2007-23-05T14:30:41Z"));
} UNIT_TEST_END


UNIT_TEST("DateTimeUtils:UTCDateTime:Validate:WrongValue2")
{
	UnitTest::Assert(!UTCDateTime::ValidateFormat("2007-01-0514:30:41Z"));
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:GenerateCurrentTime")
{
	auto t = UTCDateTime::CurrentTime();

	UnitTest::Assert(UTCDateTime::ValidateFormat(t.str()));
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:CompareLess")
{
	auto t1 = UTCDateTime("2007-01-05T14:30:41Z");
	auto t2 = UTCDateTime("2007-01-05T17:30:41Z");

	UnitTest::Assert(t1 < t2);
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:CompareMore")
{
	auto t1 = UTCDateTime("2007-01-05T14:30:41Z");
	auto t2 = UTCDateTime("2007-01-05T17:30:41Z");

	UnitTest::Assert(t2 > t1);
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UnixTimestampNow")
{
	UnitTest::Assert(UnixTimestampNow() > 1);	// #TODO write a better test
} UNIT_TEST_END

#endif	// UNIT_TESTS