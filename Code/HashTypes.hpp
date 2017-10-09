#pragma once
#include "TypeDefs.hpp"
#include <array>

// SHA-2
using hash_SHA256 = std::array<byte, 32>;
using hash_SHA512 = std::array<byte, 64>;

// SHA-3
using hash_SHA3_256 = std::array<byte, 32>;
using hash_SHA3_384 = std::array<byte, 48>;
using hash_SHA3_512 = std::array<byte, 64>;
