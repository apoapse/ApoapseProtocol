#pragma once
#include <string>
#include <functional>
#include <exception>

class FailException : public std::exception
{
};

class UnitTest
{
	std::function<void()> m_testCode;
	std::string m_fullName;

public:
	UnitTest(const std::string& fullName, const std::function<void()> code)
		: m_fullName(fullName)
		, m_testCode(code)
	{
	}

	bool operator== (const UnitTest& other) const
	{
		return m_fullName == other.GetFullName();
	}

	bool Run() const
	{
		try
		{
			m_testCode();
			return true;
		}
		catch (const FailException&)
		{
			return false;
		}
		catch (const std::exception&)
		{
			return false;
		}
	}

	std::string GetFullName() const
	{
		return m_fullName;
	}
};