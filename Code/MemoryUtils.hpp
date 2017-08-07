#pragma once

template <typename DERIVED, typename BASE>
DERIVED* object_static_cast_non_owning(BASE* ptr)
{
	return static_cast<DERIVED*>(ptr);
}
