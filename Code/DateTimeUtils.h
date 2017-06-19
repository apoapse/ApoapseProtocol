#pragma once
#include "TypeDefs.hpp"
#include <string>

namespace DateTimeUtils
{
	class UTCDateTime	// ISO 8601
	{
		std::string m_dateTimeStr;

	public:
		UTCDateTime(const std::string& dateTime);

		std::string str() const;

		bool operator<(const UTCDateTime& foreignDateTime) const;
		bool operator>(const UTCDateTime& foreignDateTime) const;

		static UTCDateTime CurrentTime();

		static bool ValidateFormat(const std::string& dateStr);
	};

	Int64 UnixTimestampNow();
}