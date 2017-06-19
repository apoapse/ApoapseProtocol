#pragma once
#include <string>
#include <functional>
#include <exception>

class SuccessException : public std::exception {};

class FailException : public std::exception
{
	std::string m_errorMsgStr;
public:
	FailException(const std::string& msg = "")
	{
		m_errorMsgStr = std::string(msg);
	}

	virtual const char* what() const override
	{
		return m_errorMsgStr.c_str();
	}
};

class UnitTest
{
	const std::function<void()> m_testCode;
	const std::string m_fullName;

public:
	UnitTest(const std::string& fullName, const std::function<void()> code)
		: m_fullName(fullName)
		, m_testCode(code)
	{
	}

	bool operator== (const UnitTest& other)
	{
		return m_fullName == other.GetFullName();
	}

	bool Run(std::string& returnErrorMsg) const
	{
		try
		{
			m_testCode();
			throw FailException("No unit test check reached");
		}
		catch (const SuccessException&)
		{
			return true;
		}
		catch (const FailException& e)
		{
			returnErrorMsg = std::string(e.what());
			return false;
		}
		catch (...)
		{
			returnErrorMsg = "An unexpected exception has been received";
			return false;
		}

		return false;
	}

	std::string GetFullName() const
	{
		return m_fullName;
	}

	//************************************
	// Method:    UnitTest::Fail - Calling this function from a unit test will directly report it as a failure and end the execution of the test
	// Access:    public static 
	// Parameter: const char * msg - Use it to explain why the unit test has failed. Optional but highly recommended
	//************************************
	static void Fail(const std::string& msg = "")
	{
		throw FailException(msg);
	}

	//************************************
	// Method:    UnitTest::Success - Calling this function from a unit test will directly report it as a success and end the execution of the test
	// Access:    public static 
	//************************************
	static void Success()
	{
		throw SuccessException();
	}

	//************************************
	// Method:    UnitTest::Assert - The expression define if the unit test will be marked as a success or a failure and end the execution of the test
	// Access:    public static 
	// Parameter: bool expression - Works like a normal assert
	// Parameter: const char * msg - Use it to explain why the unit test has failed. Optional but highly recommended
	//************************************
	static void Assert(bool expression, const std::string& msg = "")
	{
		if (expression)
			throw SuccessException();
		else
			throw FailException(msg);
	}
};