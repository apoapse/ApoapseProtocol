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

#include "stdafx.h"
#include "StringExtensions.h"
#include <boost/algorithm/string.hpp>
#include <regex>
#include <numeric>

void StringExtensions::trim_left(std::string& str)
{
	boost::trim_left(str);
}

void StringExtensions::trim_right(std::string& str)
{
	boost::trim_right(str);
}

void StringExtensions::split(const std::string& str, std::vector<std::string>& output, const std::string& separator)
{
	boost::split(output, str, boost::is_any_of(separator));
}

std::string StringExtensions::join(const std::vector<std::string>& input, const std::string& separator, bool trimLastSeparator)
{
	return std::accumulate(input.begin(), input.end(), std::string(), [&separator, trimLastSeparator](const std::string& a, const std::string& b)
	{
		if (trimLastSeparator)
			return a + (a.length() > 0 ? separator : "") + b;
		else
			return a + b;
    });
}

bool StringExtensions::IsOnlyAlphanumeric(const std::string& str)
{
	std::regex expr("[a-zA-Z0-9]*");
	return std::regex_match(str, expr);
}

std::string StringExtensions::ToLowercase(const std::string& str)
{
	return boost::algorithm::to_lower_copy(str);
}
