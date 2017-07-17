#pragma once
#include <vector>
#include "TypeDefs.hpp"
#include <optional>
#include "SQLValue.hpp"

#define COLUMN_NAME_HELPER(_columnName, _index)	_index

class SQLRow
{
	std::vector<SQLValue> m_values;

public:
	SQLRow(size_t expectedColumnCount = 0)
	{
		if (expectedColumnCount > 0)
			m_values.reserve(expectedColumnCount);
	}

	void AddValue(const SQLValue& value)
	{
		m_values.push_back(value);
	}

	const SQLValue& operator[](size_t index) const
	{
		return m_values.at(index);
	}

	size_t ColumnCount() const
	{
		return m_values.size();
	}

	// For range based loop
	std::vector<SQLValue>::const_iterator begin() const
	{
		return m_values.begin();
	}

	std::vector<SQLValue>::const_iterator end() const
	{
		return m_values.end();
	}
};

class SQLPackagedResult
{
	std::optional<std::vector<SQLRow>> m_rows;
	bool m_isSucessfull;

public:
	SQLPackagedResult() : m_isSucessfull(false)
	{
	}

	SQLPackagedResult(bool isSucessfull) : m_isSucessfull(isSucessfull)
	{
		if (isSucessfull)
			m_rows = std::vector<SQLRow>();	// Initialize the vector for boost::optional
	}

	void AddRow(const SQLRow& row)
	{
		m_rows->push_back(row);
	}

	const SQLRow& operator[](size_t rowIndex) const
	{
		if (rowIndex > RowCount() - 1)
			throw std::out_of_range("The requested row index is higher than there are rows in this column");

		return m_rows->at(rowIndex);
	}

	explicit operator bool() const
	{
		return m_isSucessfull;
	}

	size_t RowCount() const
	{
		return (m_rows.has_value()) ? m_rows->size() : 0;
	}

	// For range based loop
	std::vector<SQLRow>::const_iterator begin() const
	{
		return m_rows->begin();
	}

	std::vector<SQLRow>::const_iterator end() const
	{
		return m_rows->end();
	}
};