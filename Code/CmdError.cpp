#include "stdafx.h"
#include "CmdError.h"
#include "Common.h"
#include "Uuid.h"
#include "CommandsManager.h"
#include "GenericConnection.h"

CommandInfo& CmdError::GetInfo() const
{
	static auto info = CommandInfo();
	info.command = CommandId::error_msg;
	info.fields =
	{
		CommandField{ "error_code", FieldRequirement::ANY_MENDATORY },
		CommandField{ "related_item", FieldRequirement::ANY_OPTIONAL, FIELD_VALUE_VALIDATOR(std::vector<byte>, Uuid::IsValid) },
	};

	return info;
}

void CmdError::SendError(ApoapseErrorCode error, INetworkSender& destination, class Uuid* relatedElement /*= nullptr*/)
{
	MessagePackSerializer serializer;

	if (relatedElement != nullptr)
	{
		serializer.Group("",
		{
			MSGPK_ORDERED_APPEND(serializer, "error", (UInt16)error),
			MSGPK_ORDERED_APPEND(serializer, "related_item",  relatedElement->GetAsByteVector()),
		});
	}
	else
	{
		serializer.Group("",
		{
			MSGPK_ORDERED_APPEND(serializer, "error", (UInt16)error),
		});
	}

	Send(serializer, destination);
}

APOAPSE_COMMAND_REGISTER(CmdError, CommandId::error_msg);