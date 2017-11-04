#pragma once

struct IUser
{
	virtual const Username& GetUsername() const = 0;
	virtual const Uuid& GetUsergroup() const = 0;
	virtual void SetUsergroup(const Uuid& usergroupUuid) = 0;

	virtual ~IUser() = default;
};