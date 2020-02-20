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
#include <string>

namespace StringExtensions
{
	//************************************
	// Method:    StringExtensions::erase_left - Erase all the characters that are located at the right of the specified character
	// Returns:   void
	// Parameter: std::basic_string<T> & str - input string (direcly edited)
	// Parameter: const char delimiter - delimiter character (not erased)
	// Parameter: bool trimDelimiter - define if the delimiter character should be erased or not
	//************************************
	template <typename T>
	inline void erase_left(std::basic_string<T>& str, const char delimiter, bool trimDelimiter = false)
	{
		const auto result = str.find(delimiter);

		if (result != std::string::npos)
		{
			const size_t trimOffset = (trimDelimiter) ? 1 : 0;	// With an offset of 1, we also remove the delimiter
			str.erase(0, result + trimOffset);	// #OPTIMIZATION use erase or substr?
		}
	}

	//************************************
	// Method:    StringExtensions::erase_right - Erase all the characters that are located at the left of the specified character starting from the end of the string
	// Returns:   void
	// Parameter: std::basic_string<T> & str - input string (direcly edited)
	// Parameter: const char delimiter - delimiter character (not erased by default)
	// Parameter: bool trimDelimiter - define if the delimiter character should be erased or not
	//************************************
	template <typename T>
	inline void erase_right(std::basic_string<T>& str, const char delimiter, bool trimDelimiter = false)
	{
		const size_t result = str.find_last_of(delimiter);

		if (result != std::string::npos)	// #OPTIMIZATION use erase or substr?
		{
			const size_t trimOffset = (trimDelimiter) ? 0 : 1;	// With an offset of 1, we keep the delimiter in the string
			str = str.substr(0, result + trimOffset);
		}
	}

	//************************************
	// Method:    StringExtensions::erase_all - Erase the occurences of the specified character
	// Returns:   void
	// Parameter: std::basic_string<T> & str - input string (direcly edited)
	// Parameter: const char character - character to be removed
	//************************************
	template <typename T>
	inline void erase_all(std::basic_string<T>& str, const char character)
	{
		str.erase(std::remove(str.begin(), str.end(), character), str.end());
	}

	void trim_left(std::string& str);

	void trim_right(std::string& str);

	void split(const std::string& str, std::vector<std::string>& output, const std::string& separator);

	std::string join(const std::vector<std::string>& input, const std::string& separator, bool trimLastSeparator = false);

	template <typename T>
	inline std::basic_string<T> get_last_chars(const std::basic_string<T>& str, size_t nb_of_chars)
	{
		return str.substr(str.length() - nb_of_chars);
	}

	template <typename T, typename T_SEARCH>
	inline bool contains(const std::basic_string<T>& str, const T_SEARCH& toSearch)
	{
		return (str.find(toSearch) != std::string::npos);
	}

	//************************************
	// Method:    StringExtensions::IsOnlyAlphanumeric - Check if a string contains only alphanumerical characters or not
	// Returns:   bool
	// Parameter: const std::basic_string<T> & str - input string
	//************************************
	bool IsOnlyAlphanumeric(const std::string& str);

	std::string ToLowercase(const std::string& str);
}