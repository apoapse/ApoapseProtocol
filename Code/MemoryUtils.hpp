#pragma once
#include "Diagnostic.hpp"

template <typename DERIVED, typename BASE>
DERIVED* object_static_cast_non_owning(BASE* ptr)
{
	return static_cast<DERIVED*>(ptr);
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