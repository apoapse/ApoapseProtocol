#pragma once
#include <cryptopp/sha.h>
#include <cryptopp/hmac.h>
#include <cryptopp/secblock.h>
#include "HashTypes.hpp"

using HMAC_key = CryptoPP::SecBlock<byte>;

namespace Cryptography::HMAC
{
	inline hash_SHA256 Sign_SHA256(const HMAC_key& key, const std::vector<byte>& message)
	{
		CryptoPP::HMAC<CryptoPP::SHA256> hmac(key.data(), key.size());

		hash_SHA256 signature;
		hmac.CalculateDigest(signature.data(), message.data(), message.size());

		return signature;
	}

	inline bool VerifySignature_SHA256(const HMAC_key& key, const hash_SHA256& signature, const std::vector<byte>& message)
	{
		CryptoPP::HMAC<CryptoPP::SHA256> hmac(key.data(), key.size());
		return hmac.VerifyDigest(signature.data(), message.data(), message.size());
	}
}