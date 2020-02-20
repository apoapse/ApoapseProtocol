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

#include "stdafx.h"
#include "ProtectedByteContainer.h"
#include "Common.h"
#include <random>
#include "Hash.hpp"
#include "Diagnostic.hpp"

ProtectedByteContainer::ProtectedByteContainer(const std::vector<byte>& inputContainer)
	: m_orignalSize(inputContainer.size())
{
	m_container.reserve(m_orignalSize * 3);
	m_hash = Cryptography::SHA512(inputContainer);

	GenerateContainer(inputContainer);
}

std::vector<byte> ProtectedByteContainer::Read()
{
	std::vector<byte> output;
	output.reserve(m_orignalSize);

	int hashIncrementPos = 0;
	int containerIncrementPos = 0;
	for (size_t i = 0; i < m_orignalSize; i++)
	{
		const int spacing = ComputeSpacingFromByte(m_hash[hashIncrementPos]);

		containerIncrementPos += spacing;
		output.push_back(m_container[containerIncrementPos]);
		containerIncrementPos++;

		++hashIncrementPos;
		if (hashIncrementPos > (m_hash.size() - 1))
			hashIncrementPos = 0;
	}

	ASSERT(output.size() == m_orignalSize);

	// #TODO Re-regenerate the internal container

	return output;
}

void ProtectedByteContainer::GenerateContainer(const std::vector<byte>& inputContainer)
{
	int incrementPos = 0;
	for (const auto& cByte : inputContainer)
	{
		const int spacing = ComputeSpacingFromByte(m_hash[incrementPos]);

		if (spacing > 0)
		{
			std::vector<byte> spacingBytes(spacing);
			std::for_each(spacingBytes.begin(), spacingBytes.end(), [](byte& cByte)
			{
				cByte = GenerateRandomByte();
			});

			m_container.insert(m_container.end(), spacingBytes.begin(), spacingBytes.end());
		}

		m_container.push_back(cByte);

		++incrementPos;
		if (incrementPos > (m_hash.size() - 1))
			incrementPos = 0;
	}
}

UInt8 ProtectedByteContainer::ComputeSpacingFromByte(byte hashByte)
{
	if ((UInt8)hashByte >= 252)
		return 98;

	else if ((UInt8)hashByte < 42)
		return 2;

	else
		return 0;
}

byte ProtectedByteContainer::GenerateRandomByte()
{
	static std::random_device device;
	static std::mt19937 engine(device());
	std::uniform_int_distribution<UInt32> distrib(0, 255);

	return static_cast<byte>(distrib(engine));
}
