#pragma once
#include "TypeDefs.hpp"
#include <string>
#include "ICustomDataType.hpp"

namespace DateTimeUtils
{
	class UTCDateTime : public ICustomDataType	// ISO 8601
	{
		std::string m_dateTimeStr;

	public:
		UTCDateTime(const std::string& dateTime);
		UTCDateTime() = default;

		std::string GetStr() const override;

		bool operator<(const UTCDateTime& foreignDateTime) const;
		bool operator>(const UTCDateTime& foreignDateTime) const;

		static UTCDateTime CurrentTime();

		static bool ValidateFormat(const std::string& dateStr);
	};

	Int64 UnixTimestampNow();
}