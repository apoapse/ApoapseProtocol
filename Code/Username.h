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
