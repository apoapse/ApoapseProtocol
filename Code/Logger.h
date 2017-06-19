#pragma once
#include "Log.h"
#include "TypeDefs.hpp"
#include <chrono>
#include <memory>

#define MAX_ALLOWED_CONSECUTIVE_LOGS 12	// #TODO: expose to the settings

class Logger : public ILogger
{
	const std::string m_logFile;
	std::stringstream m_stream;
	const bool m_asyncLogToFile;
	std::unique_ptr<class ThreadPool> m_localThreadPool;

#ifdef ENABLE_SPAM_PREVENTION
	bool m_logLock;
	int m_lockedLogCount;
	std::chrono::system_clock::time_point m_previousLogRecord;
	std::chrono::system_clock::time_point m_previousLockRelease;
#endif // ENABLE_SPAM_PREVENTION

public:
	//************************************
	// Method:    Logger::Logger
	// Access:    public 
	// Parameter: const string & logFile - relative path to the file where the log message are written
	// Parameter: bool asyncLogToFile - if set to true, writing to the log file is made asynchronously
	//************************************
	Logger(const std::string& logFile, bool asyncLogToFile = true);

	void Log(const LogMessage& logMessage) override;
	bool IsSpamPreventionEngaged() const override;

private:
	static void LogToConsole(const LogMessage& logMessage);
	void LogToFile(const LogMessage& logMessage);
	static void Trace(const std::string& msg);

#ifdef _WIN32
	static UInt16 GetConsoleColorBySeverity(LogSeverity severity);
#endif

#ifdef ENABLE_SPAM_PREVENTION
	void SpamPreventionUpdate();
#endif // ENABLE_SPAM_PREVENTION
};