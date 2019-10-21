#include "stdafx.h"
#include "Common.h"
#include "JsonFileSettings.h"
#include "FileUtils.h"

JsonFileSettings::JsonFileSettings(const std::string& filePath) : m_filePath(filePath)
{
	const auto jsonData = FileUtils::ReadFile(filePath);
	std::string jsonTxt = std::string(jsonData.begin(), jsonData.end());

	// We clean the json file content first 
	jsonTxt = std::regex_replace(jsonTxt, std::regex("\\n"), "");
	jsonTxt = std::regex_replace(jsonTxt, std::regex("\\r"), "");
	jsonTxt = std::regex_replace(jsonTxt, std::regex("\\t"), "");
	
	settings = JsonHelper(jsonTxt);
}

void JsonFileSettings::SaveToFile() const
{
	const std::string jsonTxt = GetJson();
	
	const std::vector<byte> bytes(jsonTxt.begin(), jsonTxt.end());
	
	FileUtils::SaveBytesToFile(m_filePath, bytes);
}

std::string JsonFileSettings::GetJson() const
{
	const std::string jsonTxt = settings.Generate();
	std::string finalJson = jsonTxt;

	// Hack used to add quotes around string values that where directly read from the json file
	const std::regex expr("\":((?!false)(?!true)[^(?!\")].*?)[,|}]");
	std::smatch match;
	std::string::const_iterator searchStart(jsonTxt.cbegin());
	
    while (std::regex_search(searchStart, jsonTxt.cend(), match, expr))
	{
		finalJson = std::regex_replace(finalJson, std::regex(match[1].str()), "\"" + match[1].str() + "\"");
    	searchStart = match.suffix().first;
	}

	return finalJson;
}
