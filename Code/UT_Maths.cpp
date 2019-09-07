#include "stdafx.h"

#include "UnitTestsManager.h"
#include "Maths.hpp"
#include "TypeDefs.hpp"

UNIT_TEST("Maths:IsInBound:1")
{
	CHECK(IsInBound(980, 0, 1000));
} UNIT_TEST_END

UNIT_TEST("Maths:IsInBound:2")
{
	CHECK(IsInBound(-500, -1000, 0));
} UNIT_TEST_END

UNIT_TEST("Maths:IsInBound:Failure")
{
	CHECK(!IsInBound(500, 0, 10));
} UNIT_TEST_END

UNIT_TEST("Maths:CanFit")
{
	CHECK(CanFit<UInt8>((int)250));
} UNIT_TEST_END

UNIT_TEST("Maths:CanFit:Negative")
{
	CHECK(CanFit<Int16>((int)-560));
} UNIT_TEST_END

UNIT_TEST("Maths:CanFit:Failure")
{
	CHECK(!CanFit<UInt8>((int)-10));
} UNIT_TEST_END