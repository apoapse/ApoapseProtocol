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
		throw std::runtime_error("Unable to find the requested file to read");
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
