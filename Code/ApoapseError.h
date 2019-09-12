#pragma once
#include "CustomDataType.h"
#include "INetworkSender.h"
#include "ApoapseErrors.hpp"
#include <optional>
#include "Uuid.h"

class ApoapseError
{
public:
	ApoapseErrors m_error = ApoapseErrors::undefined;
	std::optional<Uuid> m_relatedItem;
	
	ApoapseError(ApoapseErrors error, INetworkSender* destination, const Uuid* relatedItem = nullptr);
	explicit ApoapseError(class CommandV2& cmd);

	std::string GetErrorStr() const;
};
