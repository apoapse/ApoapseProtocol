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
#include <memory>
#include "TypeDefs.hpp"
#include "Diagnostic.hpp"
#include "Logger.h"
#include "ThreadPool.h"
#include "MemoryUtils.hpp"
#include "Database.hpp"
#include "ApoapseData.h"
#include "ICommandManager.hpp"
#include "IHtmlUI.hpp"

struct Global
{
	bool isServer = false;
	bool isClient = false;

	std::unique_ptr<ThreadPool> threadPool;
	std::unique_ptr<ThreadPool> mainThread;
	
	std::unique_ptr<Logger> logger;
	IDatabase* database = nullptr;
	
	std::unique_ptr<ApoapseData> apoapseData;
	std::unique_ptr<ICommandManager> cmdManager;

#ifdef APOAPSE_SERVER
	class ServerSettings* settings = nullptr;
#endif // APOAPSE_SERVER

	IHtmlUI* htmlUI = nullptr;

	Global() = default;
	static Global* CreateGlobal()
	{
		return new Global();
	}
};
extern Global* global;

// Logs
void FatalError(const std::string& msg);
#define LOG if (global->logger.get() != nullptr) Log(*global->logger) << "[" << ExtractClassName(__FUNCTION__) << "] "

#ifdef DEBUG
#define LOG_DEBUG	LOG << LogSeverity::debug
#else
#define LOG_DEBUG DummyLog()
#endif


#ifdef DEBUG
#define LOG_DEBUG_FUNCTION_NAME()	LOG << LogSeverity::debug << __FUNCTION__
#else
#define LOG_DEBUG_FUNCTION_NAME()
#endif