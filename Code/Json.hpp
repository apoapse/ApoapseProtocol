#pragma once
#include <vector>
#include <sstream>
#include <string>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class JsonHelper
{
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

	template <typename T>
	void InsertArray(const std::string& path, const std::vector<T>& values)
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

	template <typename T>
	boost::optional<T> ReadFieldValue(const std::string& name) const
	{
		return m_properties.get_optional<T>(name);
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
			output.push_back(JsonHelper(item.second));
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
};
