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
#include <array>
#include <vector>
#include "TypeDefs.hpp"
#include "Range.hpp"
#include "ICustomDataType.hpp"

constexpr size_t uuidLengthInBytes = 16;

class Uuid final : public ICustomDataType
{
	std::array<byte, uuidLengthInBytes> m_uuidRawFormat;

public:
	Uuid() = default;

	Uuid(std::array<byte, uuidLengthInBytes>& uuidBytes);
	Uuid(const std::vector<byte>& uuidBytes);
	Uuid(Range<std::vector<byte>>& range);

	const std::array<byte, uuidLengthInBytes>& GetInRawFormat() const;
	std::vector<byte> GetBytes() const override;

	bool operator==(const Uuid& other) const;
	bool operator<(const Uuid& other) const;

	static Uuid Generate();

	static bool IsValid(const std::vector<byte>& range);
	static bool IsValidRange(const Range<std::vector<byte>>& range);
	
private:
};