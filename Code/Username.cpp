#include "stdafx.h"
#include "Username.h"
#include "Common.h"
#include "ByteUtils.hpp"

Username::Username(const hash_SHA256& hash) : m_usernameHash(hash)
{
}

Username::Username(const std::vector<byte>& hash)
{
	SECURITY_ASSERT(hash.size() == sha256Length);
	m_usernameHash = hash_SHA3_256();

	std::copy(hash.begin(), hash.end(), m_usernameHash->begin());
}

const hash_SHA256& Username::GetRaw() const
{
	return m_usernameHash.value();
}

ByteContainer Username::GetBytes() const
{
	const auto& val = m_usernameHash.value();
	return ByteContainer(val.begin(), val.end());
}

std::string Username::ToStr() const
{
	return (m_usernameHash.has_value()) ? BytesToHexString(m_usernameHash.value()) : ""s;
}

bool Username::operator<(const Username& other) const
{
	return (m_usernameHash.value() < other.GetRaw());
}

bool Username::operator==(const Username& other) const
{
	return (m_usernameHash.value() == other.GetRaw());
}

bool Username::operator!=(const Username& other) const
{
	return (m_usernameHash.value() != other.GetRaw());
}

bool Username::IsValid(const std::vector<byte>& usernameHash)
{
	return (usernameHash.size() == sha256Length);
}
