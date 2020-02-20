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
#include "TypeDefs.hpp"

enum class ApoapseErrors : UInt16
{
	undefined = 0,
	//network_message_too_long = 1,
	//internal_server_error = 2,
	invalid_cmd = 3,
	unknown_cmd = 4,
	//wrong_network_payload_length = 5,
	//cannot_process_cmd_from_this_connection_type = 6,
	protocol_version_not_supported = 7,
	unable_to_register_user = 8,
	unable_to_authenticate_user = 9,
	access_denied = 10,
	//future_datetime = 10,
	//related_item_does_not_exist = 11,
};