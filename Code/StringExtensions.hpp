#pragma once
#include <string>
#include <boost/algorithm/string.hpp>
#include <regex>

namespace StringExtensions
{
	//************************************
	// Method:    StringExtensions::erase_left - Erase all the characters that are located at the right of the specified character
	// Returns:   void
	// Parameter: std::basic_string<T> & str - input string (direcly edited)
	// Parameter: const char delimiter - delimiter character (not erased)
	//************************************
	template <typename T>
	void erase_left(std::basic_string<T>& str, const char delimiter)
	{
		const auto result = str.find(delimiter);

		if (result != std::string::npos)
			str.erase(0, result);	// #OPTIMIZATION use erase or substr?
	}

	//************************************
	// Method:    StringExtensions::erase_right - Erase all the characters that are located at the left of the specified character starting from the end of the string
	// Returns:   void
	// Parameter: std::basic_string<T> & str - input string (direcly edited)
	// Parameter: const char delimiter - delimiter character (not erased)
	//************************************
	template <typename T>
	void erase_right(std::basic_string<T>& str, const char delimiter)
	{
		const size_t result = str.find_last_of(delimiter);

		if (result != std::string::npos)	// #OPTIMIZATION use erase or substr?
			str = str.substr(0, result + 1);	// Offset of 1 because we want to keep the delimiter in string
	}

	//************************************
	// Method:    StringExtensions::erase_all - Erase the occurences of the specified character
	// Returns:   void
	// Parameter: std::basic_string<T> & str - input string (direcly edited)
	// Parameter: const char character - character to be removed
	//************************************
	template <typename T>
	void erase_all(std::basic_string<T>& str, const char character)
	{
		str.erase(std::remove(str.begin(), str.end(), character), str.end());
	}

	template <typename T>
	void trim_left(std::basic_string<T>& str)
	{
		boost::trim_left(str);
	}

	template <typename T>
	void trim_right(std::basic_string<T>& str)
	{
		boost::trim_right(str);
	}

	template <typename T>
	void split(const std::basic_string<T>& str, std::vector<std::basic_string<T>>& output, const std::string& separator)
	{
		boost::split(output, str, boost::is_any_of(separator));
	}

	template <typename T>
	std::basic_string<T> get_last_chars(const std::basic_string<T>& str, size_t nb_of_chars)
	{
		return str.substr(str.length() - nb_of_chars);
	}

	template <typename T>
	bool contains(const std::basic_string<T>& str, char character)
	{
		const size_t result = str.find(character);

		if (result != std::string::npos)
			return true;
		else
			return false;
	}

	//************************************
	// Method:    StringExtensions::IsOnlyAlphanumeric - Check if a string contains only alphanumerical characters or not
	// Returns:   bool
	// Parameter: const std::basic_string<T> & str - input string
	//************************************
	template <typename T>
	bool IsOnlyAlphanumeric(const std::basic_string<T>& str)
	{
		std::regex expr("[a-zA-Z0-9]*");
		return std::regex_match(str, expr);
	}
}