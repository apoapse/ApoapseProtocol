#pragma once
#include "Common.h"
#include "INetworkSender.h"
#include "CommandsManager.h"
#include "ApoapseErrorCodes.hpp"
#include "CmdError.h"

class ApoapseError
{
public:
	inline static void SendError(ApoapseErrorCode error, INetworkSender& destination)
	{
		ASSERT(error != ApoapseErrorCode::undefined);

		CmdError cmd;
		cmd.SendError(error, destination);
		LogError(error, destination);
	}

	template <typename T>
	inline static void SendError(ApoapseErrorCode error, INetworkSender& destination, const Uuid& relatedItem)
	{
		ASSERT(error != ApoapseErrorCode::undefined);

		CmdError cmd;
		cmd.SendError(error, destination, &relatedItem);
		LogError(error, destination);
	}

private:
	static void LogError(ApoapseErrorCode error, INetworkSender& destination)
	{
		LOG << LogSeverity::warning << "Send error " << (UInt16)error << " to " << destination.GetEndpoint();
	}
};