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

	inline std::string GenerateRandomCharacters(int minLength, int maxLength, bool includeSpecialChars = true)
	{
		std::string output;
		const std::vector<byte> bytes = GenerateRandomBytes(maxLength * 2);

		for (const byte cByte : bytes)
		{
			// We make sure to use only certain ascii chars/bytes from the original random bytes string
			if (includeSpecialChars)
			{
				if (cByte != 0x22 && cByte != 0x27 && (IsInBound<byte>(cByte, 0x21, 0x3B) || IsInBound<byte>(cByte, 0x41, 0x5A) || IsInBound<byte>(cByte, 0x61, 0x7A)))
					output += cByte;
			}
			else
			{
				if (IsInBound<byte>(cByte, 0x30, 0x39) || IsInBound<byte>(cByte, 0x41, 0x5A) || IsInBound<byte>(cByte, 0x61, 0x7A))
					output += cByte;
			}

			if (output.size() >= maxLength)
				return output;
		}

		if (output.size() < minLength)
			return GenerateRandomCharacters(minLength, maxLength, includeSpecialChars);
		else
			return output;
	}
}