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
#include "Uuid.h"
#include "Common.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "MemoryUtils.hpp"

Uuid::Uuid(std::array<byte, uuidLengthInBytes>& uuidBytes)
{
	m_uuidRawFormat = std::move(uuidBytes);
}

Uuid::Uuid(const std::vector<byte>& uuidBytes)
{
	if (uuidBytes.size() != uuidLengthInBytes)
		throw std::out_of_range("The input vector is not of the correct size to create a uuid");

	m_uuidRawFormat = VectorToArray<byte, uuidLengthInBytes>(uuidBytes);
}

Uuid::Uuid(Range<std::vector<byte>>& range)
{
	if (range.size() < uuidLengthInBytes)
		throw std::out_of_range("No enough bytes to create a uuid");

	std::copy(range.begin(), range.end(), m_uuidRawFormat.begin());
	range.Consume(uuidLengthInBytes);
}

const std::array<byte, uuidLengthInBytes>& Uuid::GetInRawFormat() const
{
	return m_uuidRawFormat;
}

std::vector<byte> Uuid::GetBytes() const
{
	std::vector<byte> output(m_uuidRawFormat.begin(), m_uuidRawFormat.end());

	return output;
}

bool Uuid::operator==(const Uuid& other) const
{
	return std::equal(m_uuidRawFormat.begin(), m_uuidRawFormat.end(), other.GetInRawFormat().begin());
}

bool Uuid::operator<(const Uuid& other) const
{
	return (this->GetInRawFormat() < other.GetInRawFormat());
}

Uuid Uuid::Generate()
{
	static boost::uuids::random_generator generator;
	auto uuid = generator();

	std::array<byte, 16> bytes;
	std::copy(uuid.begin(), uuid.end(), bytes.begin());

	return Uuid(bytes);
}

bool Uuid::IsValid(const std::vector<byte>& vector)
{
	return (vector.size() == uuidLengthInBytes);
}

bool Uuid::IsValidRange(const Range<std::vector<byte>>& data)
{
	return (data.size() >= uuidLengthInBytes);
}
