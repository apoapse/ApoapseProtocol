#pragma once
#include "TypeDefs.hpp"

enum class ApoapseErrorCode : UInt16
{
	undefined = 0,
	network_message_too_long = 1,
	internal_server_error = 2,
	invalid_cmd = 3,
	unknown_cmd = 4,
	wrong_network_payload_length = 5,
	cannot_processs_cmd_from_this_connection_type = 6,
	protocol_version_not_supported = 7,
};

// See SecurityAlert.cpp to associate an error code to a security alert