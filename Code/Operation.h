#pragma once
#include "Database.hpp"
#include "Uuid.h"
#include "AbstractUser.h"
#include <optional>
#include "Username.h"
#include "INetworkSender.h"

enum class OperationType : UInt32
{
	none,
	new_user,
	new_room,
	new_thread,
	new_message,
	mark_message_as_read,
};

enum class OperationOwnership
{
	all,
	sender_user,
	usergroup,
};

class Operation
{
public:
	DbId dbId = -1;
	OperationType type = OperationType::none;
	OperationOwnership ownership = OperationOwnership::all;
	Username relatedUser;
	DbId itemDbId = -1;
	Int64 time = -1;

	Operation(OperationType type, const Username& relatedUser, std::optional<DbId> itemDbId, OperationOwnership ownership = OperationOwnership::all);
	void Save();
	static Int64 GetMostRecentOperationTime(const Username& relatedUser);
private:

};