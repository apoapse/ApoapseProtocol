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

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTest.hpp"
#include "ProtectedByteContainer.h"

UNIT_TEST("ProtectedByteContainer:Read")
{
	std::vector<byte> input;
	for (int i = 128 - 1; i >= 0; i--)
	{
		input.push_back((byte)i);
	}

	ProtectedByteContainer key(input);
	CHECK(key.Read() == input);
} UNIT_TEST_END

#endif	// UNIT_TESTS