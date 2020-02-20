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

#pragma once
#include "Username.h"
class Usergroup;

struct IUser
{
	virtual ~IUser() = default;

	virtual const Username& GetUsername() const = 0;
	virtual const Usergroup& GetUsergroup() const = 0;
	virtual bool IsUsingTemporaryPassword() const { return false; }
};