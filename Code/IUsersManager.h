#pragma once
#include "CryptographyTypes.hpp"
class Username;

struct IUsersManager
{
	virtual bool DoesUserExist(const Username& username) const = 0;
	virtual PublicKeyBytes GetUserIdentityPublicKey(const Username& username) const = 0;

	virtual ~IUsersManager() = default;
};