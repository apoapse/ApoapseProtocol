// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#include "stdafx.h"

#include "UnitTest.hpp"
#include "DateTimeUtils.h"

using namespace DateTimeUtils;

UNIT_TEST("DateTimeUtils:UTCDateTime:Validate")
{
	CHECK(UTCDateTime::ValidateFormat("2007-01-05T14:30:41Z"));
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:Validate:WrongValue")
{
	CHECK(!UTCDateTime::ValidateFormat("2007-23-05T14:30:41Z"));
} UNIT_TEST_END


UNIT_TEST("DateTimeUtils:UTCDateTime:Validate:WrongValue2")
{
	CHECK(!UTCDateTime::ValidateFormat("2007-01-0514:30:41Z"));
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:GenerateCurrentTime")
{
	auto t = UTCDateTime::CurrentTime();

	CHECK(UTCDateTime::ValidateFormat(t.GetStr()));
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:CompareLess")
{
	auto t1 = UTCDateTime("2007-01-05T14:30:41Z");
	auto t2 = UTCDateTime("2007-01-05T17:30:41Z");

	CHECK(t1 < t2);
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UTCDateTime:CompareMore")
{
	auto t1 = UTCDateTime("2007-01-05T14:30:41Z");
	auto t2 = UTCDateTime("2007-01-05T17:30:41Z");

	CHECK(t2 > t1);
} UNIT_TEST_END

UNIT_TEST("DateTimeUtils:UnixTimestampNow")
{
	CHECK(UnixTimestampNow() > 1);	// #TODO write a better test
} UNIT_TEST_END