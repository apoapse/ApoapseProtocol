#pragma once
#include <cstdint>
#include <chrono>
#include <string>

using Int8		 = std::int8_t;
using Int16		 = std::int16_t;
using Int32		 = std::int32_t;
using Int64		 = std::int64_t;

using UInt8		 = std::uint8_t;
using UInt16	 = std::uint16_t;
using UInt32	 = std::uint32_t;

using UInt64	 = std::uint64_t;

using byte		 = unsigned char;

using DbId		 = Int64;

// Literals
using namespace std::literals::string_literals;	// for the string literal 's' suffix
using namespace std::literals::chrono_literals;