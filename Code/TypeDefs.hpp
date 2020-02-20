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

#pragma once
#include <cstdint>
#include <chrono>
#include <string>
#include <vector>

using Int8			 = std::int8_t;
using Int16			 = std::int16_t;
using Int32			 = std::int32_t;
using Int64			 = std::int64_t;

using UInt8			 = std::uint8_t;
using UInt16		 = std::uint16_t;
using UInt32		 = std::uint32_t;

using UInt64		 = std::uint64_t;

using byte			 = unsigned char;
using ByteContainer	 = std::vector<byte>;

using DbId			 = Int64;
using UIId			 = Int64;

// Literals
using namespace std::literals::string_literals;	// for the string literal 's' suffix
using namespace std::literals::chrono_literals;