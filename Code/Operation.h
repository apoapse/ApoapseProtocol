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
};

class Operation
{
public:
	DbId dbId = -1;
	OperationType type = OperationType::none;
	Username user;
	DbId itemDbId = -1;
	Int64 time = -1;

	Operation(OperationType type, const Username& user, std::optional<DbId> itemDbId);
	//void SaveAndSend(INetworkSender& destination);
	void Save();
	//virtual ~Operation();
	static Int64 GetMostRecentOperationTime();
private:

};