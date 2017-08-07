#pragma once
#include "TypeDefs.hpp"

enum class ApoapseErrorCode : UInt16
{
	undefined = 0,
	network_message_too_long = 1,
	internal_server_error = 2,
	malformed_cmd = 3,
};