#include "stdafx.h"
#include "Common.h"
#include "FileUtils.h"
#include <fstream>
#include <filesystem>

void FileUtils::SaveBytesToFile(const std::string& filePath, const std::vector<byte>& data)
{
	// We create the parent directory if it does not exist
	const std::string filePathFolder = std::filesystem::path(filePath).parent_path().string();
	if (!filePathFolder.empty() && !std::filesystem::exists(filePathFolder))
	{
		std::filesystem::create_directory(filePathFolder);
	}

	// Save content
	std::ofstream writer(filePath, std::ifstream::binary);
	if (writer.is_open())
	{
		writer.write((const char*)data.data(), data.size());

		LOG << "Saved " << data.size() << " bytes on file " << filePath;
		writer.close();
	}
}

std::vector<byte> FileUtils::ReadFile(const std::string& filePath)
{
	if (!std::filesystem::exists(filePath))
	{
		LOG << LogSeverity::error << "Unable to find the requested file to read: " << filePath;
		throw std::exception("Unable to find the requested file to read");
	}

	std::vector<byte> output;
	std::ifstream inputStream(filePath, std::ios::binary | std::ios::ate);

	if (inputStream.is_open())
	{
		const size_t size = static_cast<size_t>(inputStream.tellg());
		inputStream.seekg(0);

		output.resize(size);
		inputStream.read(reinterpret_cast<char*>(output.data()), size);

		ASSERT(output.size() == size);
		inputStream.close();
	}

	return output;
}
