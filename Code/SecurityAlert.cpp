#include "stdafx.h"
#include "SecurityAlert.h"
#include "Common.h"
#include "Uuid.h"
#include "CmdError.h"
#include "CommandsManager.h"

SecurityLog::SecurityAlert SecurityLog::GetAlertFromErrorCode(ApoapseErrorCode errorCode)
{
	static const std::vector<SecurityAlert> errors
	{
		{ ApoapseErrorCode::network_message_too_long, SecurityAlert::Severity::major, "Network message too long" },
		{ ApoapseErrorCode::wrong_network_payload_length, SecurityAlert::Severity::medium, "A completed network payload is not of the expected size" },
		{ ApoapseErrorCode::unknown_cmd, SecurityAlert::Severity::minor, "The requested command does not exist" },
		{ ApoapseErrorCode::cannot_processs_cmd_from_this_connection_type, SecurityAlert::Severity::major, "The connection authentication status does not match the requested Apoapse command requirements" },
	};

	const auto result = std::find_if(errors.begin(), errors.end(), [&](const auto& item)
	{
		return (item.errorCode == errorCode);
	});

	if (result == errors.end())
		return { ApoapseErrorCode::undefined };
	else
		return *result;
}

void SecurityLog::LogAlert(ApoapseErrorCode errorCode, INetworkSender& destination, const Uuid* relatedItem /*= nullptr*/)
{
	auto alert = GetAlertFromErrorCode(errorCode);

	if (alert.errorCode == ApoapseErrorCode::undefined)
	{
		LOG << LogSeverity::warning << "No registered error for the error code " << static_cast<UInt16>(errorCode);
		return;
	}

	LOG << "SECURITY ALERT: from " << destination.GetEndpointStr() << ", alert: " << alert.msg << " (" << static_cast<UInt16>(errorCode) << ") severity: " << SeverityToStr(alert.severity) << LogSeverity::security_alert;

	if (alert.additionalAction == SecurityAlert::AdditionalAction::notify_sender)
	{
		LOG << "Security alert requesting notify to the sender" << LogSeverity::security_alert;
		NotifySender(errorCode, destination, relatedItem);
	}

	if (alert.severity >= SecurityAlert::Severity::medium)
	{
		LOG << "Severity level require closing the sender connection" << LogSeverity::security_alert;
		destination.Close();
	}
}

void SecurityLog::NotifySender(ApoapseErrorCode errorCode, INetworkSender& destination, const Uuid* relatedItem /*= nullptr*/)
{
	CmdError cmd;
	cmd.SendError(errorCode, destination, relatedItem);
}

std::string SecurityLog::SeverityToStr(SecurityAlert::Severity severity)
{
	switch (severity)
	{
		case SecurityLog::SecurityAlert::Severity::info:
			return "info";

		case SecurityLog::SecurityAlert::Severity::minor:
			return "minor";

		case SecurityLog::SecurityAlert::Severity::medium:
			return "medium";

		case SecurityLog::SecurityAlert::Severity::major:
			return "major";

		case SecurityLog::SecurityAlert::Severity::critical:
			return "critical";

		default:
			return "";
	}
}
