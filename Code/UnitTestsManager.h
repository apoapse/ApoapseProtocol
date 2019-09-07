#pragma once
#include "UnitTest.hpp"
#include "MacroUtils.hpp"
#include <vector>
#include <string>
#include "TypeDefs.hpp"

class UnitTestsManager
{
	struct TestExec
	{
		std::vector<std::string> errorMsgs;
	};
	
	std::vector<UnitTest> m_registeredUnitTests;
	std::optional<TestExec> m_currentTest;

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

	void Check(bool exp, const std::string& code);
	void Require(bool exp, const std::string& code);

private:
	void Log(const std::string& msg, ConsoleColors color = ConsoleColors::DEFAULT) const;
	void SortTests();
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
#define CHECK(_exp)	UnitTestsManager::GetInstance().Check(_exp, #_exp);
#define REQUIRE(_exp)	UnitTestsManager::GetInstance().Require(_exp, #_exp);