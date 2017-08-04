#pragma once

struct ILogger
{
	virtual ~ILogger() = default;

	virtual void Log(const struct LogMessage& logMessage) = 0;
	virtual bool IsSpamPreventionEngaged() const = 0;
};