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
#include <sstream>
#include "ILogger.hpp"
#include "eLogSeverity.hpp"
#include "TypeDefs.hpp"

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

struct DummyLog
{
	template<typename T>
	DummyLog& operator<<(const T&)
	{
		return *this;
	}
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

	Log& operator<<(const std::exception& e)
	{
		m_stream << "Exception: " << e.what();
		return *this;
	}

	Log& operator<<(std::vector<byte> item);
};