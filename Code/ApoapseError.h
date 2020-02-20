// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

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
