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