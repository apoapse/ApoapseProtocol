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

struct Global
{
	bool isServer = false;
	bool isClient = false;

	std::unique_ptr<ThreadPool> threadPool;
	std::unique_ptr<Logger> logger;
	IDatabase* database = nullptr;
	std::unique_ptr<ApoapseData> apoapseData;

#ifdef APOAPSE_SERVER
	class ServerSettings* settings = nullptr;
#endif // APOAPSE_SERVER

#ifdef APOAPSE_CLIENT
	class HTMLUI* htmlUI = nullptr;
#endif // APOAPSE_CLIENT

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
#define LOG_DEBUG	//
#endif

#ifdef DEBUG
#define LOG_DEBUG_FUNCTION_NAME()	LOG << LogSeverity::debug << __FUNCTION__
#else
#define LOG_DEBUG_FUNCTION_NAME()
#endif