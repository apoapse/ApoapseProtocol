#pragma once
#include <array>
#include <vector>
#include "TypeDefs.hpp"
#include "HashTypes.hpp"

#include <cryptopp/sha.h>
#include <cryptopp/pwdbased.h>

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

	inline hash_SHA256 PBKDF2_SHA256(const std::vector<byte>& password, const std::vector<byte>& salt, UInt32 interationsCount)
	{
		std::array<byte, CryptoPP::SHA256::DIGESTSIZE> result;

		CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf2;
		pbkdf2.DeriveKey(result.data(), result.size(), 0, password.data(), password.size(), salt.data(), salt.size(), interationsCount, 0);

		return result;
	}

	inline hash_SHA512 PBKDF2_SHA512(const std::vector<byte>& password, const std::vector<byte>& salt, UInt32 interationsCount)
	{
		std::array<byte, CryptoPP::SHA512::DIGESTSIZE> result;

		CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA512> pbkdf2;
		pbkdf2.DeriveKey(result.data(), result.size(), 0, password.data(), password.size(), salt.data(), salt.size(), interationsCount, 0);

		return result;
	}
}