#pragma once
#include <array>
#include <vector>
#include "TypeDefs.hpp"
#include "HashTypes.hpp"

#include <cryptopp/sha.h>

namespace Cryptography
{
	inline hash_SHA256 SHA256(const std::vector<byte>& data)
	{
		std::array<byte, CryptoPP::SHA256::DIGESTSIZE> result;
		CryptoPP::SHA256().CalculateDigest(result.data(), data.data(), data.size());

		return result;
	}

	inline hash_SHA512 SHA512(const std::vector<byte>& data)
	{
		std::array<byte, CryptoPP::SHA512::DIGESTSIZE> result;
		CryptoPP::SHA512().CalculateDigest(result.data(), data.data(), data.size());

		return result;
	}
}