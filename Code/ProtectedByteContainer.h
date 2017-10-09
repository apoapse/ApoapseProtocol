#pragma once
#include "TypeDefs.hpp"
#include <vector>
#include "HashTypes.hpp"

// ProtectedByteContainer
// Use this class can be used to store a private key on the RAM with a bit more safety than with a simple vector
// Each call to Read will re-generate the internal container using random spacing
class ProtectedByteContainer
{
	const size_t m_orignalSize = 0;
	std::vector<byte> m_container;
	hash_SHA512 m_hash;

public:
	ProtectedByteContainer(const std::vector<byte>& inputContainer);
	
	std::vector<byte> Read();
private:
	void GenerateContainer(const std::vector<byte>& inputContainer);
	static UInt8 ComputeSpacingFromByte(byte hashByte);
	static byte GenerateRandomByte();
};