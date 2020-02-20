// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once
#include "TypeDefs.hpp"
#include "CryptographyTypes.hpp"
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <cryptopp/osrng.h>

namespace Cryptography::AES_CBC
{
	inline std::tuple<PrivateKeyBytes, IV, std::vector<byte>> Encrypt(const std::vector<byte>& msg, UInt32 keylength = CryptoPP::AES::DEFAULT_KEYLENGTH)
	{
		CryptoPP::AutoSeededRandomPool rng;

		//Gen key
		PrivateKeyBytes key(keylength);
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

	inline std::tuple<std::vector<byte>, IV> EncryptWithKey(const std::vector<byte>& msg, const PrivateKeyBytes& key)
	{
		CryptoPP::AutoSeededRandomPool rng;

		//Gen random IV
		IV iv;
		rng.GenerateBlock(iv.data(), iv.size());

		CryptoPP::CBC_Mode<CryptoPP::AES>::Encryption encryptor;
		encryptor.SetKeyWithIV(key.data(), key.size(), iv.data(), iv.size());

		std::vector<byte> encryptedMsg(msg.size() + CryptoPP::AES::BLOCKSIZE);	// Make room for padding
		CryptoPP::ArraySink sink(encryptedMsg.data(), encryptedMsg.size());
		CryptoPP::ArraySource arrSource(msg.data(), msg.size(), true, new CryptoPP::StreamTransformationFilter(encryptor, new CryptoPP::Redirector(sink)));

		encryptedMsg.resize(sink.TotalPutLength());

		return std::make_tuple(encryptedMsg, iv);
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