#pragma once
#include "TypeDefs.hpp"

static std::string GetDataStructures()
{
	static std::string scheme = R"(
{
	"data_structures": [
		{
			"name": "rooms",
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
		}
	]
}
	)";

	return scheme;
}