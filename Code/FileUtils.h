#pragma once

namespace FileUtils
{
	void SaveBytesToFile(const std::string& filePath, const std::vector<byte>& data);
	std::vector<byte> ReadFile(const std::string& filePath);
}