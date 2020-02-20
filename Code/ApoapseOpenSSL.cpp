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

#include "stdafx.h"
#include "Common.h"
#include "ApoapseOpenSSL.h"

void ApoapseOpenSSL::GenerateAllKeys()
{
	std::system("openssl genrsa -out rootca.key 2048");
	std::system("openssl req -x509 -new -nodes -key rootca.key -days 20000 -out rootca.crt -subj /");
	std::system("openssl genrsa -out server.key 2048");
	std::system("openssl req -new -key server.key -out server.csr  -subj /");
	std::system("openssl x509 -req -in server.csr -CA rootca.crt -CAkey rootca.key -CAcreateserial -out server.crt -days 365");
	std::system("openssl dhparam -out dh2048.pem 2048");
}
