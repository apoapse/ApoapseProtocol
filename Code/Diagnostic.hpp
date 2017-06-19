#pragma once
#include <iostream>
#include <string>

#ifdef DEBUG

#define ASSERT_MSG(_exp, _msg) \
	do { \
		if (!(_exp)) { \
			std::cerr << "Assertion '" << #_exp << "' failed in " << ExtractFileName(__FILE__) << " (line " << __LINE__ << ") " << ExtractClassName(__FUNCTION__) << " - " << _msg << "]\n"; \
			std::terminate(); \
		}\
	} while (false) \

#define ASSERT(_exp) \
	do { \
		if (!(_exp)) { \
			std::cerr << "Assertion '" << #_exp << "' failed in " << ExtractFileName(__FILE__) << " (line " << __LINE__ << ") [" << ExtractClassName(__FUNCTION__) << "]\n"; \
			std::terminate(); \
		}\
	} while (false) \

inline std::string ExtractFileName(const std::string& filePath)
{
	auto pos = filePath.find_last_of("\\");

	if (pos != std::string::npos)
		return filePath.substr(pos + 1, filePath.length());

	else
		return filePath;
}

#else

#define ASSERT_MSG(_exp, _msg)	
#define ASSERT(_exp)	

#endif // DEBUG

inline std::string ExtractClassName(const std::string& fullName)
{
	auto pos = fullName.find_last_of("::");

	if (pos != std::string::npos)
	{
		auto ouput = fullName.substr(0, pos - 1);

		// 		auto lambdaSpecifierPos = fullName.find("::<lambda_");
		// 		if (lambdaSpecifierPos != std::string::npos)
		// 			ouput = ouput.substr(0, lambdaSpecifierPos);

		return ouput;
	}
	else
		return fullName;
}