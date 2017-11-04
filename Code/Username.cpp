#include "stdafx.h"
#include "Username.h"
#include "Common.h"
#include "ByteUtils.hpp"

Username::Username(const hash_SHA3_256& hash) : m_usernameHash(hash)
{
}

Username::Username(const std::vector<byte>& hash) : m_usernameHash()
{
	SECURITY_ASSERT(hash.size() == sha256Length);

	std::copy(hash.begin(), hash.end(), m_usernameHash.begin());
}

const hash_SHA3_256& Username::GetRaw() const
{
	return m_usernameHash;
}

std::string Username::ToStr() const
{
	return BytesToHexString(m_usernameHash);
}

bool Username::operator<(const Username& other) const
{
	return (m_usernameHash < other.GetRaw());
}

bool Username::operator==(const Username& other) const
{
	return (m_usernameHash == other.GetRaw());
}

bool Username::operator!=(const Username& other) const
{
	return (m_usernameHash != other.GetRaw());
}

bool Username::IsValid(const std::vector<byte>& usernameHash)
{
	return (usernameHash.size() == sha256Length);
}
