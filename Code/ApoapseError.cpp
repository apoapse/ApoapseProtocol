#include "stdafx.h"
#include "Common.h"
#include "ApoapseError.h"
#include <magic_enum.hpp>
#include "CommandsManagerV2.h"
#include "Uuid.h"

ApoapseError::ApoapseError(ApoapseErrors error, INetworkSender* destination, const Uuid* relatedItem/* = nullptr*/)
{
	m_error = error;
	
	if (global->isServer && destination)
	{
		DataStructure dat = global->apoapseData->GetStructure("error");
		dat.GetField("error").SetValue((Int64)error);

		if (relatedItem != nullptr)
		{
			m_relatedItem = *relatedItem;
			dat.GetField("related_item").SetValue(*relatedItem);
		}
		
		global->cmdManager->CreateCommand("error", dat).Send(*destination);
		destination->RequestClose();
		
		LOG << LogSeverity::error << "Sent error " << magic_enum::enum_name(error) << " to " << destination->GetEndpointStr();
	}
	else
	{
		LOG << LogSeverity::error << "Triggered error " << magic_enum::enum_name(error) << " (no recipient)";
	}
}

ApoapseError::ApoapseError(CommandV2& cmd)
{
	ASSERT(cmd.name == "error");
	ASSERT(global->isClient);

	m_error = static_cast<ApoapseErrors>(cmd.GetData().GetField("error").GetValue<Int64>());

	if (cmd.GetData().GetField("related_item").HasValue())
		m_relatedItem = cmd.GetData().GetField("related_item").GetValue<Uuid>();
}

std::string ApoapseError::GetErrorStr() const
{
	return std::string(magic_enum::enum_name(m_error));
}
