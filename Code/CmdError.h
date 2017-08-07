#pragma once
#include "Command.h"
#include <optional>
#include "ApoapseErrorCodes.hpp"

class CmdError : public Command
{
public:
	CommandInfo& GetInfo() const override;

	void SendError(ApoapseErrorCode error, INetworkSender& destination, class Uuid* relatedElement = nullptr);
	
private:
};