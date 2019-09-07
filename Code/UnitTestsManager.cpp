#include "stdafx.h"
#include "UnitTestsManager.h"
#include "Diagnostic.hpp"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

void UnitTestsManager::RunTests(const std::string& testsPath /*= ""*/)
{
	int successCount = 0;
	int errorsCount = 0;
	const auto toExecuteTestsCount = (int)m_registeredUnitTests.size();

	SortTests();

	Log("EXECUTING " + std::to_string(toExecuteTestsCount) + " UNIT TESTS...");

	for (const UnitTest& test : m_registeredUnitTests)
	{
		std::string exceptionError;
		m_currentTest = TestExec();

		const bool testResult = test.Run(exceptionError);

		if (testResult && m_currentTest->errorMsgs.empty())
		{
			Log("TEST " + test.GetFullName() + " -> SUCCESS", ConsoleColors::GREEN);
			++successCount;
		}
		else
		{
			Log("TEST " + test.GetFullName() + " -> FAILURE", ConsoleColors::RED);

			for (const std::string& errorMsg : m_currentTest->errorMsgs)
			{
				Log("\t " + errorMsg, ConsoleColors::RED);
			}

			if (!exceptionError.empty())
			{
				Log("\t Exception triggered: " + exceptionError, ConsoleColors::RED);
			}
			
			++errorsCount;
		}
	}

	const ConsoleColors resultColor = (errorsCount == 0) ? ConsoleColors::DEFAULT : ConsoleColors::RED;
	Log("EXECUTED " + std::to_string(toExecuteTestsCount) + " UNIT TESTS. " + std::to_string(successCount) + " successful, " + std::to_string(errorsCount) + " failed", resultColor);
}

void UnitTestsManager::RegisterTest(const UnitTest& test)
{
	m_registeredUnitTests.push_back(test);
}

void UnitTestsManager::Check(bool exp, const std::string& code)
{
	if (!exp)
	{
		m_currentTest->errorMsgs.push_back("CHECK failed on: " + code);
	}
}

void UnitTestsManager::Require(bool exp, const std::string& code)
{
	if (!exp)
	{
		m_currentTest->errorMsgs.push_back("REQUIRE failed on: " + code);
		throw FailException();
	}
}

void UnitTestsManager::Log(const std::string& msg, ConsoleColors color /*= ConsoleColors::DEFAULT*/) const
{
#ifdef _WIN32
	switch (color)
	{
	case ConsoleColors::GREEN:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case ConsoleColors::RED:
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	}
#endif

	std::cout << msg << '\n';

#ifdef _WIN32
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);	// Reset to default color
#endif
}

void UnitTestsManager::SortTests()
{
	std::sort(m_registeredUnitTests.begin(), m_registeredUnitTests.end(), [](UnitTest& left, UnitTest& right)
	{
		return (left.GetFullName() < right.GetFullName());
	});
}
