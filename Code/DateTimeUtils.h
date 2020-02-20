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