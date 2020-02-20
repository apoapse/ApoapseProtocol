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
#include "TypeDefs.hpp"
#include <array>
#include "Range.hpp"
#include <sstream>
#include <iomanip>

// #ifdef _WIN32
// #define BOOST_LITTLE_ENDIAN 1
// #else
// #define BOOST_ENDIAN_DEPRECATED_NAMES
// #include <boost/endian/endian.hpp>
// #endif

enum class Endianness
{
	AP_LITTLE_ENDIAN,
	AP_BIG_ENDIAN
};

template <typename T>
inline std::array<byte, sizeof(T)> ToBytes(T data, Endianness endianness = Endianness::AP_LITTLE_ENDIAN)
{
	static_assert(std::is_trivially_copyable<T>::value, "The type provided is not trivially copyable");
	std::array<byte, sizeof(T)> bytes{};

	const byte* it = static_cast<const byte*>(static_cast<void*>(std::addressof(data)));

	if (endianness == Endianness::AP_LITTLE_ENDIAN)
		std::copy(it, it + sizeof(T), bytes.begin());
	else
		std::reverse_copy(it, it + sizeof(T), bytes.begin());

	return bytes;
}

template <typename T>
inline T FromBytes(std::array<byte, sizeof(T)>& byteArray, Endianness endianness = Endianness::AP_LITTLE_ENDIAN)
{
	static_assert(std::is_trivially_copyable<T>::value, "The type provided is not trivially copyable");

	T output;

	if (endianness == Endianness::AP_LITTLE_ENDIAN)
		std::copy(byteArray.begin(), byteArray.end(), static_cast<byte*>(static_cast<void*>(std::addressof(output))));
	else
		std::reverse_copy(byteArray.begin(), byteArray.end(), static_cast<byte*>(static_cast<void*>(std::addressof(output))));

	return output;
}

template <typename T, typename RANGE_TYPE>
inline T FromBytes(Range<RANGE_TYPE>& bytes, Endianness endianness = Endianness::AP_LITTLE_ENDIAN)
{
	static_assert(std::is_trivially_copyable<T>::value, "The type provided is not trivially copyable");

	T output;

	if (endianness == Endianness::AP_LITTLE_ENDIAN)
		std::copy(bytes.begin(), bytes.begin() + sizeof(T), static_cast<byte*>(static_cast<void*>(std::addressof(output))));
	else
		std::reverse_copy(bytes.begin(), bytes.begin() + sizeof(T), static_cast<byte*>(static_cast<void*>(std::addressof(output))));

	return output;
}

template <typename T>
std::string BytesToHexString(const T& bytesArray)
{
	static_assert(std::is_same<typename T::value_type, byte>::value, "The array provided do not contain bytes");

	std::stringstream ss;
	ss << std::hex << std::setfill('0');

	for (const auto& byte : bytesArray)
	{
		ss << static_cast<Int16>(byte);
	}

	return ss.str();
}