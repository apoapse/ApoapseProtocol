#pragma once
#include <vector>

template <typename T>
class Range
{
	const std::vector<T>& m_associatedArray;
	size_t m_cursorPosition = 0;
	size_t m_upperBound = 0;

public:
	Range(const std::vector<T>& array, size_t upperBound = 0)
		: m_associatedArray(array)
		, m_upperBound(upperBound)
	{
	}

	void Consume(size_t length)
	{
		m_cursorPosition += length;

		if (m_cursorPosition > m_associatedArray.size())
			throw std::out_of_range("");
	}

	T operator[](size_t pos) const
	{
		return m_associatedArray[m_cursorPosition + pos];
	}

	size_t Size() const
	{
		return (m_associatedArray.size() - m_upperBound) - m_cursorPosition;	// No risk of integer overflow as Consume() would have trown an exception if m_cursorPosition become higher than the array actual size
	}

	size_t GetCursorPosition() const
	{
		return m_cursorPosition;
	}

	typename std::vector<T>::const_iterator begin() const
	{
		return m_associatedArray.begin() + m_cursorPosition;
	}

	typename std::vector<T>::const_iterator end() const
	{
		return m_associatedArray.end() - m_upperBound;
	}
};