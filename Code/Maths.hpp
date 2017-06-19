#pragma once
#include <limits>

template <typename T>
constexpr bool IsInBound(T value, T low, T high)
{
	return (value >= low) && (value <= high);
}

//************************************
// Method:    CanFit - Check if a number can fit into a specified integer type 
// Access:    public 
// Returns:   constexpr bool
// Parameter: size_t value - 
//************************************
template <typename TARGET_TYPE, typename T>
constexpr bool CanFit(T value)
{
	return IsInBound((Int64)value, (Int64)std::numeric_limits<TARGET_TYPE>::min(), (Int64)std::numeric_limits<TARGET_TYPE>::max());
}