#include "stdafx.h"
#include "DateTimeUtils.h"
#include <boost/date_time/posix_time/posix_time.hpp>
#include <regex>
#include <chrono>
#include "Common.h"

DateTimeUtils::UTCDateTime::UTCDateTime(const std::string& dateTime) : m_dateTimeStr(dateTime)
{
	if (!ValidateFormat(dateTime))
		throw std::runtime_error("Text of unexpected format for a ISO-8601 date time");
}

std::string DateTimeUtils::UTCDateTime::GetStr() const
{
	return m_dateTimeStr;
}

bool DateTimeUtils::UTCDateTime::operator<(const UTCDateTime& foreignDateTime) const
{
	return (GetStr() < foreignDateTime.GetStr());
}

bool DateTimeUtils::UTCDateTime::operator>(const UTCDateTime& foreignDateTime) const
{
	return (GetStr() > foreignDateTime.GetStr());
}

DateTimeUtils::UTCDateTime DateTimeUtils::UTCDateTime::CurrentTime()
{
	using namespace boost::posix_time;
	ptime t = second_clock::universal_time();

	auto test = to_iso_extended_string(t);

	return UTCDateTime(to_iso_extended_string(t) + "Z");
}

bool DateTimeUtils::UTCDateTime::ValidateFormat(const std::string& dateStr)
{
	if (dateStr.length() != 20)
		return false;

	std::regex expr("[0-9]{4}-[0-1][0-9]-[0-3][0-9]T[0-2][0-9]:[0-5][0-9]:[0-5][0-9]Z");
	return std::regex_match(dateStr, expr);
}

Int64 DateTimeUtils::UnixTimestampNow()
{
	return static_cast<Int64>(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}
