#pragma once
#include "Json.hpp"

class JsonFileSettings
{
	std::string m_filePath;
	
public:
	JsonHelper settings;

	JsonFileSettings() = default;
	JsonFileSettings(const std::string& filePath);
	
	void SaveToFile() const;
	std::string GetJson() const;
};
