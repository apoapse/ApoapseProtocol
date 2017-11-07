#pragma once
#include "TypeDefs.hpp"

enum class CommandId : UInt16
{
	error_msg							= 1,
	connect								= 2,
	unit_test_cmd						= 3,
	server_info							= 4
};