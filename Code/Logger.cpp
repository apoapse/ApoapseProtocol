#include "stdafx.h"
#include "Logger.h"
#include <iostream>
#include <fstream>
#include "Diagnostic.hpp"
#include "ThreadPool.h"

#ifdef _WIN32
#include <windows.h>
#endif

Logger::Logger(const std::string& logFile, bool asyncLogToFile /*= true*/)
	: m_logFile(logFile),
	m_asyncLogToFile(asyncLogToFile)
{
	if (m_asyncLogToFile)
		m_localThreadPool = std::make_unique<ThreadPool>("Logger (" + m_logFile + ")", 1);
}

void Logger::Log(const LogMessage& logMessage)
{
#ifdef ENABLE_SPAM_PREVENTION
	SpamPreventionUpdate();
	if (IsSpamPreventionEngaged())
		return;
#endif // ENABLE_SPAM_PREVENTION

	LogToConsole(logMessage);

	if (m_asyncLogToFile)
	{
		ASSERT(m_localThreadPool.get() != nullptr);
		m_localThreadPool->PushTask([this, logMessage] { LogToFile(logMessage); });
	}
	else
		LogToFile(logMessage);
}

void Logger::LogToConsole(const LogMessage& logMessage)
{
#ifdef DEBUG
	Trace(logMessage.severityPrefix + logMessage.msg);
#endif

#ifdef _WIN32
	// Warning: There are no thread synchronization here so sometimes when there are a lot of calls from multiple threads the colors can be out of place
	// but it's a minor annoyance compared to slow down the whole program
	auto color = GetConsoleColorBySeverity(logMessage.logSeverity);

	if (color != 7)	//	Only if it's not to apply the default color again
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);

	std::cout << logMessage.msg << '\n';

	if (color != 7)
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);	// Reset to default color

#else
	PLATFORM_NOT_IMPLEMENTED("unix console colors");
	std::cout << steam.rdbuf() << '\n';
#endif
}

void Logger::LogToFile(const LogMessage& logMessage)
{
// 	try
// 	{
		std::ofstream ofs;
		ofs.open(m_logFile, std::ofstream::out | std::ofstream::app);

		if (!ofs.is_open())
			return;

		ofs << logMessage.dateTime << " - " << logMessage.severityPrefix << logMessage.msg << '\n';

		ofs.close();
// 	}
// 	catch (const std::exception&)
// 	{
// 		//#TODO Handle error
// 	}
}

#ifdef ENABLE_SPAM_PREVENTION
void Logger::SpamPreventionUpdate()
{
	const UInt32 maxAllowedConsecutiveLogs = MAX_ALLOWED_CONSECUTIVE_LOGS;

	//const auto now = std::chrono::system_clock::now();
	const auto now = std::chrono::steady_clock::now();
	const std::chrono::milliseconds previousLogDifference = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_previousLogRecord);
	++m_lockedLogCount;

	if (previousLogDifference.count() <= 160 && m_lockedLogCount >= maxAllowedConsecutiveLogs)
	{
		if (!m_logLock)
		{
			//LogMessage warningMsg("Log spam prevention engaged: too many log calls in a short period of time", LogSeverity::warning);
			//warningMsg.LogToConsole();
			//warningMsg.LogToFile();

			m_logLock = true;
		}
	}
	else
	{
		if (m_logLock)
		{
			//LogMessage warningMsg(Format("Log spam prevention released (approximately %1% log calls blocked)", (m_lockedLogCount - maxAllowedConsecutiveLogs)), LogSeverity::normal);
			//warningMsg.LogToConsole();
			//warningMsg.LogToFile();

			m_logLock = false;
			m_lockedLogCount = 0;
		}
	}

	m_previousLogRecord = now;
}
#endif // ENABLE_SPAM_PREVENTION


bool Logger::IsSpamPreventionEngaged() const
{
#ifdef ENABLE_SPAM_PREVENTION
	return m_logLock;
#else
	return false;
#endif // ENABLE_SPAM_PREVENTION
}


#ifdef _WIN32
UInt16 Logger::GetConsoleColorBySeverity(LogSeverity severity)
{
	switch (severity)
	{
	case LogSeverity::debug:
		return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;	//	Cyan

	case LogSeverity::warning:
		return 14;															//	Yellow

	case LogSeverity::error:
	case LogSeverity::fatalError:
		return FOREGROUND_RED | FOREGROUND_INTENSITY;						// Red

	case LogSeverity::security_alert:
		return FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY;

	default:
		return 7;															//	Default color (light gray)
	}
}
#endif

void Logger::Trace(const std::string& msg)
{
#ifdef _WIN32
	std::wostringstream os_;
	os_ << msg.c_str() << '\n';
	OutputDebugStringW(os_.str().c_str());
#endif
}