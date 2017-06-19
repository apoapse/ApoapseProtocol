#pragma once
#include "TypeDefs.hpp"
#include <array>

#ifdef _WIN32
#define BOOST_LITTLE_ENDIAN 1
#else
#define BOOST_ENDIAN_DEPRECATED_NAMES
#include <boost/endian/endian.hpp>
#endif

enum class Endianness
{
	LITTLE_ENDIAN,
	BIG_ENDIAN
};

#ifdef BOOST_LITTLE_ENDIAN
template <typename T, Endianness ENDIANNESS = Endianness::LITTLE_ENDIAN>
#else
template <typename T, Endianness ENDIANNESS = Endianness::BIG_ENDIAN>
#endif
inline std::array<byte, sizeof(T)> ToBytes(T data)
{
	static_assert(std::is_trivially_copyable<T>::value, "The type provided is not trivially copyable");
	std::array<byte, sizeof(T)> bytes;

	const byte* it = static_cast<const byte*>(static_cast<void*>(std::addressof(data)));

	if (ENDIANNESS == Endianness::LITTLE_ENDIAN)
		std::copy(it, it + sizeof(T), std::begin(bytes));
	else
		std::reverse_copy(it, it + sizeof(T), std::begin(bytes));

	return bytes;
}

#ifdef BOOST_LITTLE_ENDIAN
template <typename T, Endianness ENDIANNESS = Endianness::LITTLE_ENDIAN>
#else
template <typename T, Endianness ENDIANNESS = Endianness::BIG_ENDIAN>
#endif
inline T FromBytes(std::array<byte, sizeof(T)>& byteArray)
{
	static_assert(std::is_trivially_copyable<T>::value, "The type provided is not trivially copyable");

	T output;

	if (ENDIANNESS == Endianness::LITTLE_ENDIAN)
		std::copy(std::begin(byteArray), std::end(byteArray), static_cast<byte*>(static_cast<void*>(std::addressof(output))));
	else
		std::reverse_copy(std::begin(byteArray), std::end(byteArray), static_cast<byte*>(static_cast<void*>(std::addressof(output))));

	return output;
}
