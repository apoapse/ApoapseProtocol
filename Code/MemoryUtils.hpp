#pragma once
#include "Diagnostic.hpp"
#include <array>

template <typename DERIVED, typename BASE>
inline DERIVED* object_static_cast_non_owning(BASE* ptr)
{
	return static_cast<DERIVED*>(ptr);
}

template <typename T, size_t SIZE>
inline std::array<T, SIZE> VectorToArray(const std::vector<T>& vector)
{
	if (vector.size() != SIZE)
		throw std::range_error("The specified vector is not of the expect length");

	std::array<T, SIZE> output;
	std::copy(vector.begin(), vector.end(), output.begin());

	return output;
}

template <typename T, size_t SIZE>
inline std::vector<T> ArrayToVector(const std::array<T, SIZE>& arr)
{
	std::vector<T> output;
	output.reserve(SIZE);

	std::copy(arr.begin(), arr.end(), std::back_inserter(output));
	return output;
}

template <typename CLASS>
class RawPtrOwner final
{
	CLASS* m_ptr;

public:
	~RawPtrOwner()
	{
		if (m_ptr != nullptr)
			delete m_ptr;
	}

	RawPtrOwner(CLASS* ptr)
	{
		m_ptr = ptr;
	}

	RawPtrOwner() = default;
	RawPtrOwner(const RawPtrOwner&) = delete;
	RawPtrOwner& operator=(const RawPtrOwner&) = delete;

	RawPtrOwner& operator=(CLASS* ptr)
	{
		m_ptr = ptr;
		return *this;
	}

	CLASS* operator->() noexcept
	{
		ASSERT(m_ptr != nullptr);

		return m_ptr;
	}

	CLASS* Get() const noexcept
	{
		return m_ptr;
	}

	explicit operator bool() const
	{
		return (m_ptr != nullptr);
	}
};