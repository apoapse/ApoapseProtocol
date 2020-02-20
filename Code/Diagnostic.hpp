// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once
#include <iostream>
#include <string>

class ExceptionDynMsg : public std::exception
{
	const std::string m_msg;

public:
	ExceptionDynMsg(const std::string& msg) : m_msg(msg)
	{
	}

	virtual char const* what() const noexcept override
	{
		return m_msg.c_str();
	}
};

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

#else

#define ASSERT_MSG(_exp, _msg)
#define ASSERT(_exp)

#endif // DEBUG

#ifdef ENABLE_SECURITY_ASSERTS

#define SECURITY_ASSERT(_exp)	if (!(_exp))	throw ExceptionDynMsg(std::string("SECURITY ASSERT [" + ExtractClassName(__FUNCTION__) + ":" + std::to_string(__LINE__) + "] " + #_exp));

#else

#define SECURITY_ASSERT(_exp)	

#endif // ENABLE_SECURITY_ASSETS

inline std::string ExtractFileName(const std::string& filePath)
{
	auto pos = filePath.find_last_of("\\");

	if (pos != std::string::npos)
		return filePath.substr(pos + 1, filePath.length());

	else
		return filePath;
}

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