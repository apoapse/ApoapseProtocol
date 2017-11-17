#pragma once
#include "CryptographyTypes.hpp"
#include <vector>
#include <cryptopp/osrng.h>

namespace Cryptography
{
	inline std::vector<byte> GenerateRandomBytes(size_t length)
	{
		std::vector<byte> output(length);
		CryptoPP::AutoSeededRandomPool rng;

		rng.GenerateBlock(output.data(), length);

		return output;
	}
}