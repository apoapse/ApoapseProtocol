#pragma once
#include "TypeDefs.hpp"

static std::string GetCommandDef()
{
	static std::string scheme = R"(

{
	"commands":	[
		{
			"name": "install",
			"name_short": "in",
			"datastructure": "install",
			"require_authentication": false,
			"only_non_authenticated": true,
			"propagate_to_other_clients": false,
			"operation": {
				"register": false
			}
		},
		{
			"name": "login",
			"name_short": "lc",
			"datastructure": "client_login",
			"require_authentication": false,
			"only_non_authenticated": true,
			"propagate_to_other_clients": false,
			"operation": {
				"register": false
			}
		},
		{
			"name": "create_room",
			"name_short": "cr",
			"datastructure": "rooms",
			"require_authentication": true,
			"propagate_to_other_clients": true,
			"save_operation": true,
			"propagate_to_client_ui": true,
			"operation": {
				"register": true,
				"ownership": "all"
			}
		}
	]
}

)";

	return scheme;
}