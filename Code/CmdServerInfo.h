#pragma once
#include "Command.h"

class CmdServerInfo : public Command
{
public:
	CommandInfo& GetInfo() const override
	{
		static auto info = CommandInfo();
		info.command = CommandId::server_info;
		info.clientOnly = true;
		info.onlyNonAuthenticated = true;
		info.fields =
		{
			CommandField{ "status", FieldRequirement::any_mendatory, FIELD_VALUE_VALIDATOR(std::string, CmdServerInfo::ValidateStatusField) }
		};

		return info;
	}

	void SendSetupState(INetworkSender& destination);

private:
	static bool ValidateStatusField(const std::string& str)
	{
		return (str == "authenticated" || str == "setup_state");
	}
};