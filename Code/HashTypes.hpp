#pragma once
#include "TypeDefs.hpp"
#include <array>

static constexpr UInt32 sha256Length = 32;
static constexpr UInt32 sha384Length = 48;
static constexpr UInt32 sha512Length = 64;

// SHA-2
using hash_SHA256 = std::array<byte, sha256Length>;
using hash_SHA512 = std::array<byte, sha512Length>;

// SHA-3
using hash_SHA3_256 = std::array<byte, sha256Length>;
using hash_SHA3_384 = std::array<byte, sha384Length>;
using hash_SHA3_512 = std::array<byte, sha512Length>;
