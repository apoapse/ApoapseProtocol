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