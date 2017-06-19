#pragma once
#include "UnitTest.hpp"
#include "MacroUtils.hpp"
#include <vector>
#include <string>

class UnitTestsManager
{
	std::vector<UnitTest> m_registeredUnitTests;

	enum class ConsoleColors
	{
		DEFAULT,
		GREEN,
		RED
	};

	UnitTestsManager() = default;

public:
	void RunTests(const std::string& testsPath = "");
	void RegisterTest(const UnitTest& test);

	UnitTestsManager(UnitTestsManager const&) = delete;
	void operator=(UnitTestsManager const&) = delete;

	static UnitTestsManager& GetInstance()
	{
		static UnitTestsManager testsManager;
		return testsManager;
	}

private:
	void Log(const std::string& msg, ConsoleColors color = ConsoleColors::DEFAULT) const;
};

class UnitTestAutoRegister
{
public:
	UnitTestAutoRegister(const UnitTest& test)
	{
		UnitTestsManager::GetInstance().RegisterTest(test);
	}
};

#define UNIT_TEST(_name)	static UnitTestAutoRegister MACRO_CONCAT(testRegister_, __COUNTER__)(UnitTest(_name, []() -> void
#define UNIT_TEST_END		));