#include "stdafx.h"

#ifdef UNIT_TESTS

#include "Common.h"
#include "UnitTest.hpp"
#include "HMAC.hpp"
#include "RSA.hpp"
#include "Hash.hpp"
#include "AES.hpp"
#include "Random.hpp"

UNIT_TEST("Cryptography:HMAC_SHA256")
{
	auto orignalkey = Cryptography::SHA256(std::vector<byte>{0xff, 0x03});
	auto key = HMAC_key(orignalkey.data(), orignalkey.size());
	std::vector<byte> message{ 0x01, 0x02, 0x08, 0x09 };

	auto hmacSignature = Cryptography::HMAC::Sign_SHA256(key, message);

	CHECK(Cryptography::HMAC::VerifySignature_SHA256(key, hmacSignature, message));
} UNIT_TEST_END

UNIT_TEST("Cryptography:RSA:EncryptDecrypt")
{
	auto[privateKey, publicKey] = Cryptography::RSA::GenerateKeyPair(1024);
	const std::string message = "Hello";

	auto cipher = Cryptography::RSA::Encrypt(publicKey, std::vector<byte>(message.begin(), message.end()));
	auto result = Cryptography::RSA::Decrypt(privateKey, cipher);

	CHECK(std::string(result.begin(), result.end()) == message);
} UNIT_TEST_END

UNIT_TEST("Cryptography:RSA:Sign")
{
	const std::string message = "Hello";
	auto[privateKey, publicKey] = Cryptography::RSA::GenerateKeyPair(1024);

	auto signature = Cryptography::RSA::Sign(privateKey, std::vector<byte>(message.begin(), message.end()));

	CHECK(Cryptography::RSA::VerifySignature(signature, publicKey, std::vector<byte>(message.begin(), message.end())));
} UNIT_TEST_END

UNIT_TEST("Cryptography:AES_CBC:EncryptDecrypt")
{
	std::vector<byte> plain{ 0x01, 0x02, 0x03, 0x04 };

	const auto[key, iv, encryptedMsg] = Cryptography::AES_CBC::Encrypt(plain);

	auto decryptedMsg = Cryptography::AES_CBC::Decrypt(key, iv, encryptedMsg);

	CHECK(decryptedMsg == plain);
} UNIT_TEST_END

UNIT_TEST("Cryptography:Random:GenerateRandomCharacters:fixed")
{
	const std::string txt = Cryptography::GenerateRandomCharacters(12, 12);
	
	CHECK(txt.length() == 12);
} UNIT_TEST_END

UNIT_TEST("Cryptography:Random:GenerateRandomCharacters:minLength")
{
	for (int i = 0; i < 100; ++i)
	{
		const std::string txt = Cryptography::GenerateRandomCharacters(5, 12);
	
		CHECK(txt.length() >= 5);
		CHECK(txt.length() <= 12);
	}
} UNIT_TEST_END

UNIT_TEST("Cryptography:Random:GenerateRandomCharacters:noSpecialChar")
{
	for (int i = 0; i < 300; ++i)
	{
		const std::string txt = Cryptography::GenerateRandomCharacters(10, 25, false);
	
		CHECK_PRINT(StringExtensions::IsOnlyAlphanumeric(txt), txt);
	}
} UNIT_TEST_END


#endif	// UNIT_TESTS