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
#include "Log.h"
#include <boost/date_time.hpp>
#include "ByteUtils.hpp"

LogMessage::LogMessage(const std::string& message, LogSeverity severity)
	: msg(message)
	, logSeverity(severity)
{
	GenerateDateTime();	// The time is recorded as soon as possible in order to not loose accuracy once the message is registered in the log file via an async call
	GenerateSeverityPrefix();
}

void LogMessage::GenerateSeverityPrefix()
{
	switch (logSeverity)
	{
	case LogSeverity::debug:
		severityPrefix = "DEBUG: ";
		break;

	case LogSeverity::warning:
		severityPrefix = "WARNING: ";
		break;

	case LogSeverity::error:
		severityPrefix = "ERROR: ";
		break;

	case LogSeverity::fatalError:
		severityPrefix = "FATAL ERROR: ";
		break;

	case LogSeverity::security_alert:
		severityPrefix = "SECURITY ALERT: ";
		break;

	default:
		severityPrefix = "";
		break;
	}
}

void LogMessage::GenerateDateTime()
{
	using boost::posix_time::ptime;
	using namespace boost::gregorian;

	std::stringstream stream;

	const ptime currentDate = boost::posix_time::second_clock::local_time();
	date date(currentDate.date());

	date_facet* facet(new date_facet("%a, %d %b %Y "));
	stream.imbue(std::locale(std::locale::classic(), facet));
	stream << date << currentDate.time_of_day();

	dateTime = stream.str();
}

Log& Log::operator<<(std::vector<byte> item)
{
	m_stream << BytesToHexString(item);
	return *this;
}
