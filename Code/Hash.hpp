#pragma once
#include <array>
#include <vector>
#include "TypeDefs.hpp"
#include "HashTypes.hpp"

#include <cryptopp/sha.h>
#include <cryptopp/sha3.h>
#include <cryptopp/pwdbased.h>

namespace Cryptography
{
	// SHA-2
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

	// SHA-3
	inline hash_SHA3_256 SHA3_256(const std::vector<byte>& data)
	{
		std::array<byte, CryptoPP::SHA3_256::DIGESTSIZE> result;

		CryptoPP::SHA3_256().CalculateDigest(result.data(), data.data(), data.size());

		return result;
	}

	inline hash_SHA3_384 SHA3_384(const std::vector<byte>& data)
	{
		std::array<byte, CryptoPP::SHA3_384::DIGESTSIZE> result;

		CryptoPP::SHA3_384().CalculateDigest(result.data(), data.data(), data.size());

		return result;
	}

	inline hash_SHA3_512 SHA3_512(const std::vector<byte>& data)
	{
		std::array<byte, CryptoPP::SHA3_512::DIGESTSIZE> result;

		CryptoPP::SHA3_512().CalculateDigest(result.data(), data.data(), data.size());

		return result;
	}

	// PBKDF-2
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