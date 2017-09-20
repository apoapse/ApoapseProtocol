#pragma once
#include "INetworkSender.h"
#include "ApoapseErrorCodes.hpp"

class Uuid;

class SecurityLog
{
	struct SecurityAlert
	{
		enum class Severity : UInt16
		{
			info,
			minor,
			medium,
			major,
			critical
		};

		enum class AdditionalAction
		{
			none,
			notify_sender
		};

		ApoapseErrorCode errorCode;
		Severity severity;
		std::string msg {};
		AdditionalAction additionalAction = AdditionalAction::none;
	};

public:
	static void LogAlert(ApoapseErrorCode errorCode, INetworkSender& destination, const Uuid* relatedItem = nullptr);

private:
	static SecurityAlert GetAlertFromErrorCode(ApoapseErrorCode errorCode);
	static void NotifySender(ApoapseErrorCode errorCode, INetworkSender& destination, const Uuid* relatedItem = nullptr);
	static std::string SeverityToStr(SecurityAlert::Severity severity);
};