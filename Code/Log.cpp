#include "Log.h"
#include <boost\date_time.hpp>

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

	ptime currentDate = boost::posix_time::second_clock::local_time();
	date date(currentDate.date());

	date_facet* facet(new date_facet("%a, %d %b %Y "));
	stream.imbue(std::locale(std::locale::classic(), facet));
	stream << date << currentDate.time_of_day();

	dateTime = stream.str();
}
