#pragma once
#include "TypeDefs.hpp"
#include "CryptographyTypes.hpp"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

namespace Cryptography::AES_CBC
{
	inline std::tuple<PrivateKeyBytes, IV, std::vector<byte>> Encrypt(const std::vector<byte>& msg)
	{
		CryptoPP::AutoSeededRandomPool rng;

		//Gen key
		PrivateKeyBytes key(CryptoPP::AES::DEFAULT_KEYLENGTH);
		rng.GenerateBlock(key.data(), key.size());

		//Gen random IV
		IV iv;
		rng.GenerateBlock(iv.data(), iv.size());

		CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
		encryptor.SetKeyWithIV(key.data(), key.size(), iv.data(), iv.size());

		std::vector<byte> encryptedMsg(msg.size() + CryptoPP::AES::BLOCKSIZE);	// Make room for padding
		CryptoPP::ArraySink sink(encryptedMsg.data(), encryptedMsg.size());
		CryptoPP::ArraySource arrSource(msg.data(), msg.size(), true, new CryptoPP::StreamTransformationFilter(encryptor, new CryptoPP::Redirector(sink)));

		encryptedMsg.resize(sink.TotalPutLength());

		return std::make_tuple(key, iv, encryptedMsg);
	}

	inline std::vector<byte> Decrypt(const PrivateKeyBytes& key, const IV& iv, const std::vector<byte>& encryptedMsg)
	{
		CryptoPP::CBC_Mode<CryptoPP::AES>::Decryption decryptor;
		decryptor.SetKeyWithIV(key.data(), key.size(), iv.data(), iv.size());

		std::vector<byte> decryptedMsg(encryptedMsg.size());	// The recovered text is always smaller than the cipher text
		CryptoPP::ArraySink sink(decryptedMsg.data(), decryptedMsg.size());
		CryptoPP::ArraySource arrSource(encryptedMsg.data(), encryptedMsg.size(), true, new CryptoPP::StreamTransformationFilter(decryptor, new CryptoPP::Redirector(sink)));

		decryptedMsg.resize(sink.TotalPutLength());

		return decryptedMsg;
	}
}