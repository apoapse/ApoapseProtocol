#pragma once
#include "TypeDefs.hpp"
#include <array>

namespace Cryptography
{
	using hash_SHA256 = std::array<byte, 32>;
	using hash_SHA512 = std::array<byte, 64>;
}
