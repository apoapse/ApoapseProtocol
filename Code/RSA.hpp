#pragma once
#include <vector>
#include "TypeDefs.hpp"
#include "CryptographyTypes.hpp"
#include <cryptopp/sha.h>
#include <cryptopp/rsa.h>
#include <cryptopp/seed.h>
#include <cryptopp/osrng.h>


namespace Cryptography
{
	class RSA
	{
	public:
		inline static std::pair<PrivateKeyBytes, PublicKeyBytes> GenerateKeyPair(UInt32 keySize)
		{
			CryptoPP::AutoSeededRandomPool rng;

			CryptoPP::InvertibleRSAFunction rsaParams;
			rsaParams.GenerateRandomWithKeySize(rng, keySize);

			CryptoPP::RSA::PrivateKey privateKey(rsaParams);
			CryptoPP::RSA::PublicKey publicKey(rsaParams);

			if (!privateKey.Validate(rng, 3) || !publicKey.Validate(rng, 3))
				throw std::runtime_error("Failed to generate a RSA key pair");

			return std::make_pair(RSAPrivateKeyToBytes(privateKey, keySize), RSAPublicKeyToBytes(publicKey, keySize));
		}

		inline static std::vector<byte> Encrypt(const PublicKeyBytes& publicKeybytes, const std::vector<byte>& message)
		{
			const auto rsaPublicKey = BytesToPublicKeyRSA(publicKeybytes);
			CryptoPP::RSAES_OAEP_SHA_Encryptor encryptor(rsaPublicKey);

			if (message.size() > encryptor.FixedMaxPlaintextLength())
				throw std::runtime_error("The message to encrypt is too large for the RSA key provided");

			CryptoPP::AutoSeededRandomPool rng;
			const size_t ciphertextLength = encryptor.CiphertextLength(message.size());

			std::vector<byte> encryptedMsg(ciphertextLength);
			encryptor.Encrypt(rng, message.data(), message.size(), encryptedMsg.data());

			return encryptedMsg;
		}

		inline static std::vector<byte> Decrypt(const PrivateKeyBytes& privateKeyBytes, const std::vector<byte>& encryptedMsg)
		{
			const auto rsaPrivateKey = BytesToPrivateKeyRSA(privateKeyBytes);
			CryptoPP::RSAES_OAEP_SHA_Decryptor decryptor(rsaPrivateKey);

			if (encryptedMsg.size() != decryptor.FixedCiphertextLength())
				throw std::runtime_error("The encrypted message is not equal to the fixed cipher length for the private key provided");

			CryptoPP::AutoSeededRandomPool rng;
			const size_t decryptedMsgMaxLength = decryptor.MaxPlaintextLength(encryptedMsg.size());

			std::vector<byte> decryptedMsg(decryptedMsgMaxLength);
			auto result = decryptor.Decrypt(rng, encryptedMsg.data(), encryptedMsg.size(), decryptedMsg.data());

			if (!result.isValidCoding)
				throw std::runtime_error("Unable to decode the message with the provided private key");

			decryptedMsg.resize(result.messageLength);

			return decryptedMsg;
		}

		inline static std::vector<byte> Sign(const PrivateKeyBytes& privateKeyBytes, const std::vector<byte>& message)
		{
			const auto rsaPrivateKey = BytesToPrivateKeyRSA(privateKeyBytes);
			CryptoPP::RSASSA_PKCS1v15_SHA_Signer signer(rsaPrivateKey);

			const size_t signatureMaxLength = signer.MaxSignatureLength();
			std::vector<byte> signatureBytes(signatureMaxLength);

			CryptoPP::AutoSeededRandomPool rng;
			const size_t signatureLength = signer.SignMessage(rng, message.data(), message.size(), signatureBytes.data());

			signatureBytes.resize(signatureLength);

			return signatureBytes;
		}

		inline static bool VerifySignature(const std::vector<byte>& signatureBytes, const PublicKeyBytes& publicKeyBytes, const std::vector<byte>& message)
		{
			const auto rsaPublicKey = BytesToPublicKeyRSA(publicKeyBytes);
			CryptoPP::RSASSA_PKCS1v15_SHA_Verifier verifier(rsaPublicKey);

			return verifier.VerifyMessage(message.data(), message.size(), signatureBytes.data(), signatureBytes.size());
		}

	private:
		static inline PrivateKeyBytes RSAPrivateKeyToBytes(const CryptoPP::RSA::PrivateKey& rsaPrivateKey, UInt32 keySize)
		{
			std::vector<byte> sinkBuffer(keySize);
			CryptoPP::ArraySink sink(sinkBuffer.data(), sinkBuffer.size());
			rsaPrivateKey.DEREncode(sink);

			return PrivateKeyBytes(sinkBuffer.data(), sink.TotalPutLength());
		}

		static inline PublicKeyBytes RSAPublicKeyToBytes(const CryptoPP::RSA::PublicKey& rsaPublicKey, UInt32 keySize)
		{
			std::vector<byte> sinkBuffer(keySize);
			CryptoPP::ArraySink sink(sinkBuffer.data(), sinkBuffer.size());
			rsaPublicKey.DEREncode(sink);

			return PublicKeyBytes(sinkBuffer.begin(), sinkBuffer.begin() + sink.TotalPutLength());
		}

		static inline CryptoPP::RSA::PublicKey BytesToPublicKeyRSA(const PublicKeyBytes& publicKeybytes)
		{
			CryptoPP::RSA::PublicKey rsaPublicKey;

			CryptoPP::ArraySource source(publicKeybytes.data(), publicKeybytes.size(), true);
			rsaPublicKey.BERDecode(source);

			return rsaPublicKey;
		}

		static inline CryptoPP::RSA::PrivateKey BytesToPrivateKeyRSA(const PrivateKeyBytes& privateKeybytes)
		{
			CryptoPP::RSA::PrivateKey rsaPrivateKey;

			CryptoPP::ArraySource source(privateKeybytes.data(), privateKeybytes.size(), true);
			rsaPrivateKey.BERDecode(source);

			return rsaPrivateKey;
		}
	};
}