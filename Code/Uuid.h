#pragma once
#include <array>
#include <vector>
#include "TypeDefs.hpp"
#include "Range.hpp"

constexpr size_t uuidLengthInBytes = 16;

class Uuid
{
	std::array<byte, uuidLengthInBytes> m_uuidRawFormat;

public:
	Uuid() = default;

	Uuid(std::array<byte, uuidLengthInBytes>& uuidBytes);
	Uuid(const std::vector<byte>& uuidBytes);
	Uuid(Range<std::vector<byte>>& range);

	const std::array<byte, uuidLengthInBytes>& GetInRawFormat() const;
	std::vector<byte> GetAsByteVector() const;

	bool operator==(const Uuid& other);

	static Uuid Generate();

	static bool IsValid(const std::vector<byte>& range);
	static bool IsValidRange(const Range<std::vector<byte>>& range);
	
private:
};