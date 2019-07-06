#pragma once
#include "TypeDefs.hpp"

static std::string GetDataStructures()
{
	static std::string scheme = R"(
{
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
					"name": "item",
					"type": "uuid",
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
					"required": true,
					"unique": false,
					"uses":	{
						"server_storage": true,
						"client_storage": true,
						"command": false
					}
				},				
				{
					"name": "related_user",
					"type": "username",
					"required": true,
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
						"command": false
					}
				},
				{
					"name": "password",
					"type": "blob",
					"required": false,
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
					"required": false,
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
						"command": false
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