#pragma once
#include <string>
#include "CryptographyTypes.hpp"

class Username
{
	hash_SHA3_256 m_usernameHash;

public:
	Username() = default;
	Username(const hash_SHA3_256& hash);
	Username(const std::vector<byte>& hash);

	const hash_SHA3_256& GetRaw() const;
	std::string ToStr() const;

	friend std::ostream& operator<<(std::ostream& stream, const Username& username)
	{
		stream << username.ToStr();
		return stream;
	}

	bool operator<(const Username& other) const
	{
		return (m_usernameHash < other.GetRaw());
	}

	bool operator==(const Username& other) const
	{
		return (m_usernameHash == other.GetRaw());
	}

	static bool IsValid(const std::vector<byte>& usernameHash);
};