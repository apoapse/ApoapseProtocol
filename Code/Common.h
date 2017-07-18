#pragma once
#include <string>
#include <memory>
#include "TypeDefs.hpp"
#include "Diagnostic.hpp"
#include "Logger.h"
#include "ThreadPool.h"

struct Global
{
	//std::unique_ptr<ISettingsManager> settings;
	std::unique_ptr<class ThreadPool> threadPool;
	std::unique_ptr<Logger> logger;

	// Only defined on the server
	class ServerSettings* settings = nullptr;

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

#define SECURITY_ASSERT(_exp)	ASSERT_MSG(_exp, "Security assert")	// #TODO design so that we can use it outside of the debug mode