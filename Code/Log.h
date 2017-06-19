#pragma once
#include <string>
#include <sstream>
#include "ILogger.hpp"
#include "eLogSeverity.hpp"

struct LogMessage
{
	const std::string msg;
	const LogSeverity logSeverity;
	std::string severityPrefix;
	std::string dateTime;

	LogMessage(const std::string& message, LogSeverity severity);

private:
	void GenerateSeverityPrefix();
	void GenerateDateTime();
};

class Log
{
	std::stringstream m_stream;
	ILogger& m_logger;
	LogSeverity m_logSeverity{ LogSeverity::normal };

public:
	Log(ILogger& logger) : m_logger(logger)
	{
	}

	virtual ~Log()
	{
		m_logger.Log(LogMessage(m_stream.str(), m_logSeverity));
	}

	Log& operator<<(LogSeverity logSeverity)
	{
		m_logSeverity = logSeverity;
		return *this;
	}

	template<typename T>
	Log& operator<<(const T& item)
	{
		m_stream << item;
		return *this;
	}

	Log& operator<<(bool item)
	{
		m_stream << std::string((item) ? "true" : "false");
		return *this;
	}
};