#include "stdafx.h"

#include "UnitTestsManager.h"
#include "Maths.hpp"
#include "TypeDefs.hpp"

UNIT_TEST("Maths:IsInBound:1")
{
	UnitTest::Assert(IsInBound(980, 0, 1000));
} UNIT_TEST_END

UNIT_TEST("Maths:IsInBound:2")
{
	UnitTest::Assert(IsInBound(-500, -1000, 0));
} UNIT_TEST_END

UNIT_TEST("Maths:IsInBound:Failure")
{
	UnitTest::Assert(!IsInBound(500, 0, 10));
} UNIT_TEST_END

UNIT_TEST("Maths:CanFit")
{
	UnitTest::Assert(CanFit<UInt8>((int)250));
} UNIT_TEST_END

UNIT_TEST("Maths:CanFit:Negative")
{
	UnitTest::Assert(CanFit<Int16>((int)-560));
} UNIT_TEST_END

UNIT_TEST("Maths:CanFit:Failure")
{
	UnitTest::Assert(!CanFit<UInt8>((int)-10));
} UNIT_TEST_END