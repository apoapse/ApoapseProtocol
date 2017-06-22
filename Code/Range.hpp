#pragma once
template <typename T>
class Range
{
	const T& m_associatedArray;
	size_t m_cursorPosition = 0;
	size_t m_upperBound = 0;

public:
	//************************************
	// Method:    Range<T>::Range - 
	// Access:    public 
	// Parameter: const T & array - std container
	// Parameter: size_t upperBound - Upper bound of the data, starting from the begging of the container
	//************************************
	Range(const T& array, size_t upperBound = 0)
		: m_associatedArray(array)
	{
		m_upperBound = (upperBound == 0) ? m_associatedArray.size() : upperBound;
	}

	void Consume(size_t length)
	{
		m_cursorPosition += length;

		if (m_cursorPosition > m_upperBound)
			throw std::out_of_range("Can't consume more than the size of the range");
	}

	typename T::value_type operator[](size_t pos) const
	{
		return m_associatedArray[m_cursorPosition + pos];
	}

	size_t Size() const
	{
		return (m_upperBound - m_cursorPosition);	// No risk of integer overflow as Consume() would have trown an exception if m_cursorPosition become higher than the array actual size
	}

	size_t GetCursorPosition() const
	{
		return m_cursorPosition;
	}

	typename T::const_iterator begin() const
	{
		return m_associatedArray.begin() + m_cursorPosition;
	}

	typename T::const_iterator end() const
	{
		return m_associatedArray.begin() + m_upperBound;
	}
};