#pragma once
#include "TypeDefs.hpp"

static std::string GetDataStructures()
{
	static std::string scheme = R"(
{
	"custom_types": [
		{
			"name": "uuid",
			"underlying_type": "blob",
			"auto_fill": true
		},
		{
			"name": "username",
			"underlying_type": "blob"
		},
		{
			"name": "sha256",
			"underlying_type": "blob",
			"min_length": 32,
			"max_length": 32
		},
		{
			"name": "timestamp",
			"underlying_type": "integer",
			"min_length": 1
		},
		{
			"name": "datetime",
			"underlying_type": "text"
		}
	],

	"data_structures": [
		{
			"name": "error",
			"fields": [
				{
					"name": "error_code",
					"type": "integer",
					"required": true,
					"unique": false,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				},
				{
					"name": "related_item",
					"type": "uuid",
					"required": false,
					"unique": false,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				}
			]
		},
		{
			"name": "client_login",
			"fields": [
				{
					"name": "protocol_version",
					"type": "integer",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				},
				{
					"name": "username",
					"type": "username",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				},
				{
					"name": "password",
					"type": "sha256",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				}
			]
		},
		{
			"name": "server_info",
			"fields": [
				{
					"name": "status",
					"type": "text",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true,
						"client_ui": true
					}
				},
				{
					"name": "username",
					"type": "username",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true,
						"client_ui": true
					}
				},
				{
					"name": "requirePasswordChange",
					"type": "bool",
					"required": false,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				},
				{
					"name": "nickname",
					"type": "text",
					"required": false,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true,
						"client_ui": true
					}
				}
			]
		},
		{
			"name": "install",
			"fields": [
				{
					"name": "admin_username",
					"type": "username",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				},
				{
					"name": "admin_password",
					"type": "sha256",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				},
				{
					"name": "admin_nickname",
					"type": "text",
					"required": true,
					"uses":	{
						"server_storage": false,
						"client_storage": false,
						"command": true
					}
				}
			]
		},
		{
			"name": "operation",
			"fields": [
				{
					"name": "name",
					"type": "text",
					"required": true,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": false
					}
				},				
				{
					"name": "time",
					"type": "timestamp",
					"required": true,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": false
					}
				},							
				{
					"name": "ownership",
					"type": "integer",
					"required": false,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": false,
						"command": false
					}
				},				
				{
					"name": "related_user",
					"type": "username",
					"required": false,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": false,
						"command": false
					}
				},
				{
					"name": "item",
					"type": "integer",
					"required": false,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": false
					}
				}
			]
		},
		{
			"name": "user",
			"fields": [
				{
					"name": "username",
					"type": "username",
					"required": true,
					"unique": true,
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "password",
					"type": "blob",
					"required": true,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": false,
						"command": false
					}
				},
				{
					"name": "password_salt",
					"type": "blob",
					"required": true,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": false,
						"command": false
					}
				},
				{
					"name": "is_temporary_password",
					"type": "bool",
					"required": false,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": false,
						"command": false
					}
				},
				{
					"name": "nickname",
					"type": "text",
					"required": false,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				}
			]
		},
		{
			"name": "room",
			"fields": [
				{
					"name": "uuid",
					"type": "uuid",
					"required": true,
					"unique": true,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "name",
					"type": "text",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "threads_layout",
					"type": "text",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				}
			]
		},
		{
			"name": "thread",
			"fields": [
				{
					"name": "uuid",
					"type": "uuid",
					"required": true,
					"unique": true,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "parent_room",
					"type": "uuid",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "name",
					"type": "text",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				}
			]
		},
		{
			"name": "message",
			"fields": [
				{
					"name": "uuid",
					"type": "uuid",
					"required": true,
					"unique": true,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "parent_thread",
					"type": "uuid",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "message",
					"type": "text",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "sent_time",
					"type": "datetime",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				},
				{
					"name": "author",
					"type": "username",
					"required": true,
					"unique": false,
					"read_permission": "all",
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": true
					}
				}
			]
		},
		{
			"name": "local_setting",
			"fields": [
				{
					"name": "name",
					"type": "text",
					"required": true,
					"unique": true,
					"uses":	{
						"server_storage": false,
						"client_storage": true,
						"command": false
					}
				},
				{
					"name": "value",
					"type": "text",
					"required": true,
					"unique": false,
					"uses":	{
						"server_storage": false,
						"client_storage": true,
						"command": false
					}
				}
			]
		}
	]
}

)";

	return scheme;
}