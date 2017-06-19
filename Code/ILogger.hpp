#pragma once

struct ILogger
{
	virtual void Log(const struct LogMessage& logMessage) = 0;
	virtual bool IsSpamPreventionEngaged() const = 0;
};