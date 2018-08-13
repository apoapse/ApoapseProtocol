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

enum class OperationDirection : UInt8
{
	none,
	sent,
	received
};

class Operation
{
	DbId m_dbId = -1;

public:
	OperationType type = OperationType::none;
	OperationDirection direction = OperationDirection::none;
	Username user;
	std::optional<Uuid> itemUuid;
	Int64 time = -1;

	Operation(OperationType type, OperationDirection direction, const Username& user, std::optional<Uuid> itemUuid);
	//void SaveAndSend(INetworkSender& destination);
	void Save();
	//virtual ~Operation();
	
private:
	static DbId GetPreviousDbId();
};