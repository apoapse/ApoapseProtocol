#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTestsManager.h"
#include "HMAC.hpp"
#include "RSA.hpp"
#include "Hash.hpp"

UNIT_TEST("Cryptography:HMAC_SHA256")
{
	auto orignalkey = Cryptography::SHA256(std::vector<byte>{0xff, 0x03});
	auto key = HMAC_key(orignalkey.data(), orignalkey.size());
	std::vector<byte> message{ 0x01, 0x02, 0x08, 0x09 };

	auto hmacSignature = Cryptography::HMAC::Sign_SHA256(key, message);

	UnitTest::Assert(Cryptography::HMAC::VerifySignature_SHA256(key, hmacSignature, message));
} UNIT_TEST_END

UNIT_TEST("Cryptography:RSA")
{
	auto[privateKey, publicKey] = Cryptography::RSA::GenerateKeyPair(1024);
	const std::string message = "Hello";

	auto cipher = Cryptography::RSA::Encrypt(publicKey, std::vector<byte>(message.begin(), message.end()));
	auto result = Cryptography::RSA::Decrypt(privateKey, cipher);

	UnitTest::Assert(std::string(result.begin(), result.end()) == message);
} UNIT_TEST_END

UNIT_TEST("Cryptography:RSA")
{
	const std::string message = "Hello";
	auto[privateKey, publicKey] = Cryptography::RSA::GenerateKeyPair(1024);

	auto signature = Cryptography::RSA::Sign(privateKey, std::vector<byte>(message.begin(), message.end()));

	UnitTest::Assert(Cryptography::RSA::VerifySignature(signature, publicKey, std::vector<byte>(message.begin(), message.end())));
} UNIT_TEST_END

#endif	// UNIT_TESTS