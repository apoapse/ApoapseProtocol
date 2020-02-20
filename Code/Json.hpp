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
#include <vector>
#include <sstream>
#include <string>
#include <regex>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
using namespace std::literals::string_literals;

class JsonHelper
{
	struct StringTranslator
	{
		boost::optional<std::string> get_value(const std::string& str) { return  str.substr(1, str.size() - 2); }
		boost::optional<std::string> put_value(const std::string& str) { return '"' + EscapeChars(str) + '"'; }
	};

	boost::property_tree::ptree m_properties;

public:
	JsonHelper() = default;

	JsonHelper(const std::string& json)
	{
		std::stringstream stream;
		stream << json;

		boost::property_tree::read_json(stream, m_properties);
	}

	JsonHelper(boost::property_tree::ptree node)
	{
		m_properties = node;
	}

	void Insert(const std::string& path, const JsonHelper& writer)
	{
		const bool childExist = m_properties.get_child_optional(path).is_initialized();

		if (!childExist)
		{
			boost::property_tree::ptree childs;

			childs.push_back(std::make_pair("", writer.GetPropertyTree()));

			m_properties.put_child(path, childs);
		}
		else
		{
			m_properties.get_child(path).push_back(std::make_pair("", writer.GetPropertyTree()));
		}
	}

	template <typename T>
	void Insert(const std::string& path, const T& value)
	{
		m_properties.add(path, value);
	}

	template <>
	void Insert(const std::string& path, const std::string& value)
	{
		m_properties.add(path, std::string(value), StringTranslator());
	}

	void Insert(const std::string& path, const std::string& value)
	{
		m_properties.add(path, value, StringTranslator());
	}

	void Insert(const std::string& path, const char* value)
	{
		m_properties.add(path, std::string(value), StringTranslator());
	}

	template <typename T>
	void InsertArray(const std::string& path, const std::vector<T>& values)
	{
		if (values.empty())
		{
			m_properties.add(path, "[]");
		}
		else
		{
			boost::property_tree::ptree childTree;

			for (auto& value : values)
			{
				boost::property_tree::ptree arrayItem;

				arrayItem.put("", value);
				childTree.push_back(std::make_pair("", arrayItem));
			}

			m_properties.add_child(path, childTree);
		}
	}

	template <>
	void InsertArray(const std::string& path, const std::vector<std::string>& values)
	{
		if (values.empty())
		{
			m_properties.add(path, "[]");
		}
		else
		{
			boost::property_tree::ptree childTree;

			for (auto& value : values)
			{
				boost::property_tree::ptree arrayItem;

				arrayItem.put("", value, StringTranslator());
				childTree.push_back(std::make_pair("", arrayItem));
			}

			m_properties.add_child(path, childTree);
		}
	}

	template <typename T>
	boost::optional<T> ReadFieldValue(const std::string& name) const
	{
		return m_properties.get_optional<T>(name);
	}

	template <typename T>
	void EditField(const std::string& name, const T& val)
	{
		//m_properties.get<T>(name) = val;
		m_properties.put(name, val, StringTranslator());
	}

	bool ValueExist(const std::string& name) const
	{
		return (m_properties.count(name));
	}

	template <typename T>
	std::vector<T> ReadFieldArray(const std::string& name) const
	{
		std::vector<T> outputArray;
		const auto requestedField = m_properties.get_child_optional(name);

		if (requestedField.is_initialized())
		{
			for (const auto& item : requestedField.get())
				outputArray.push_back(item.second.get_value<T>());
		}

		return outputArray;
	}

	template <>
	std::vector<JsonHelper> ReadFieldArray(const std::string& name) const
	{
		std::vector<JsonHelper> output;

		for (const auto& item : m_properties.get_child(name))
		{
			auto test = item.second;
			output.emplace_back(item.second);
		}

		return output;
	}

	const boost::property_tree::ptree& GetPropertyTree() const
	{
		return m_properties;
	}

	std::string Generate() const
	{
		std::stringstream outputStream;
		boost::property_tree::write_json(outputStream, m_properties, false);

		std::string outputStr = outputStream.str();

		//There should be no line break in the json but boost has the tendency to add one at the end so we remove it
		if (outputStr[outputStr.length() - 1] == '\n')
		{
			outputStr.erase(outputStr.length() - 1, 1);
		}

		return outputStr;
	}

	private:
		static std::string EscapeChars(const std::string& str)
		{
			std::string output = str;

			output = std::regex_replace(output, std::regex(R"(\\)"), R"(\\)");
			
			output = std::regex_replace(output, std::regex("\\\\"), "\\\\");
			output = std::regex_replace(output, std::regex("\""), "\\\"");

			return output;
		}
};
