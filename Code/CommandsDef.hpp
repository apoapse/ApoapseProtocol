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
			"reception": {
				"server": true,
				"client": false
			},
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
			"reception": {
				"server": true,
				"client": false
			},
			"operation": {
				"register": false
			}
		},
		{
			"name": "server_info",
			"name_short": "si",
			"datastructure": "server_info",
			"require_authentication": false,
			"only_non_authenticated": true,
			"propagate_to_other_clients": false,
			"reception": {
				"server": false,
				"client": true
			},
			"operation": {
				"register": false
			},
			"client_ui": {
				"propagate": true,
				"signal_name": "OnReceivedServerInfo"
			}
		},
		{
			"name": "create_room",
			"name_short": "cr",
			"datastructure": "room",
			"require_authentication": true,
			"propagate_to_other_clients": true,
			"save_on_receive": true,
			"reception": {
				"server": true,
				"client": true
			},
			"operation": {
				"register": true,
				"ownership": "all"
			},
			"client_ui": {
				"propagate": true,
				"signal_name": "OnRoomCreated"
			}
		}
	]
}

)";

	return scheme;
}