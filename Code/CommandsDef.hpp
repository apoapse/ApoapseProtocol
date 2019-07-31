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
			"reception": {
				"server": false,
				"client": true
			},
			"client_ui": {
				"propagate": true,
				"signal_name": "OnReceivedServerInfo"
			}
		},
		{
			"name": "request_sync",
			"name_short": "rs",
			"datastructure": "sync",
			"require_authentication": true,
			"save_on_receive": false,
			"reception": {
				"server": true,
				"client": false
			},
			"propagation": {
				"propagate": false,
				"exclude_self": false
			}
		},
		{
			"name": "user",
			"name_short": "nu",
			"datastructure": "user",
			"require_authentication": true,
			"only_non_authenticated": false,
			"save_on_receive": true,
			"reception": {
				"server": false,
				"client": true
			},
			"operation": {
				"register": true,
				"ownership": "all"
			}
		},
		{
			"name": "create_room",
			"name_short": "cr",
			"datastructure": "room",
			"require_authentication": true,
			"save_on_receive": true,
			"reception": {
				"server": true,
				"client": true
			},
			"operation": {
				"register": true,
				"ownership": "all"
			},
			"propagation": {
				"propagate": true,
				"exclude_self": false
			}
		},
		{
			"name": "create_thread",
			"name_short": "ct",
			"datastructure": "thread",
			"require_authentication": true,
			"save_on_receive": true,
			"reception": {
				"server": true,
				"client": true
			},
			"operation": {
				"register": true,
				"ownership": "all"
			},
			"propagation": {
				"propagate": true,
				"exclude_self": false
			}
		},
		{
			"name": "new_message",
			"name_short": "cm",
			"datastructure": "message",
			"require_authentication": true,
			"save_on_receive": true,
			"reception": {
				"server": true,
				"client": true
			},
			"operation": {
				"register": true,
				"ownership": "all"
			},
			"propagation": {
				"propagate": true,
				"exclude_self": false
			}
		}
	]
}

)";

	return scheme;
}