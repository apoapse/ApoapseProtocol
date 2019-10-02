#pragma once
#include <string>
#include "CryptographyTypes.hpp"
#include <optional>
#include "ICustomDataType.hpp"

class Username : public ICustomDataType
{
	std::optional<hash_SHA3_256> m_usernameHash;

public:
	Username() = default;
	Username(const hash_SHA256& hash);
	Username(const std::vector<byte>& hash);

	const hash_SHA256& GetRaw() const;
	ByteContainer GetBytes() const override;
	std::string ToStr() const;

	friend std::ostream& operator<<(std::ostream& stream, const Username& username)
	{
		stream << username.ToStr();
		return stream;
	}

	bool operator<(const Username& other) const;
	bool operator==(const Username& other) const;
	bool operator!=(const Username& other) const;

	static bool IsValid(const std::vector<byte>& usernameHash);
};
