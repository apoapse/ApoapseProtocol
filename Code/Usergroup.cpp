#include "stdafx.h"
#include "RSA.hpp"
#include "Usergroup.h"
#include "Common.h"
#include "UsergroupsManager.h"
#include "MemoryUtils.hpp"
#include "SQLQuery.h"

Usergroup::Usergroup(const Uuid& uuid, UsergroupsManager* usergrpManager)
	: uuid(uuid)
	, usergroupsManager(usergrpManager)
{

}

Usergroup Usergroup::CreateFromDatabase(const Uuid& uuid, UsergroupsManager& usergrpManager)
{
	Usergroup group(uuid, &usergrpManager);
	group.ConstructFromDatabase();

	return group;
}

void Usergroup::ConstructFromDatabase()
{
	SQLQuery query(*global->database);
	query << SELECT << "version, block, mac, date_time" << FROM << "usergroups_blockchain" << WHERE << "uuid" << EQUALS << uuid.GetAsByteVector() << ORDER_BY << "version" << ASC;
	auto res = query.Exec();

	if (!res)
		FatalError("Corrupted database");

	if (res.RowCount() != 1)
		throw std::exception("Unable to find the requested usergroup uuid");

	UsergroupBlock* previousBlock = nullptr;
	for (const auto& row : res)
	{
		const UInt64 version = row[0].GetInt64();
		const std::string dateTimeRaw = row[3].GetText();
		const std::vector<byte> rawblock = row[1].GetByteArray();
		const std::vector<byte> mac = row[2].GetByteArray();

		{
			UsergroupBlock block{};
			block.version = version;
			block.dateTime = DateTimeUtils::UTCDateTime(dateTimeRaw);
			block.mac = mac;
			block.usergroupUuid = uuid;

			MessagePackDeserializer deserializer(rawblock);
			if (!UsergroupBlock::ReadRawBlock(deserializer, block, usergroupsManager->usersManager))
				std::exception("Invalid usergroup block");

			if (ValidateBlockInContext(block, previousBlock))
			{
				previousBlock = &block;
				m_blockchain.push_back(block);
			}
			else
			{
				throw std::exception("Invalid block");
			}
		}
	}

	LOG << "Blockchain of the usergroup " << uuid.GetAsByteVector() << " loaded with " << m_blockchain.size() << " blocks";
}

bool Usergroup::ValidateBlockInContext(const UsergroupBlock& block, const UsergroupBlock* previousBlock)
{
	const PublicKeyBytes signerPublicKey = usergroupsManager->usersManager.GetUserIdentityPublicKey(block.macSigner);
	if (!ValidateBlockMac(block, signerPublicKey))
	{
		LOG << LogSeverity::error << "The block signature is invalid";
		return false;
	}

	if (previousBlock == nullptr)
	{
		if (block.version != 1)
		{
			LOG << LogSeverity::error << "The first block does not have 1 as a version number";
			return false; // #TODO Do not disconnect the client, just discard the block
		}
	}
	else
	{
		if (block.version != (previousBlock->version - 1))
		{
			LOG << LogSeverity::error << "Wrong block version number";
			return false;
		}

		if (!ComparePreviousBlockHash(block, previousBlock))
		{
			LOG << LogSeverity::error << "The previous block hash indicated in the current block does not match the actual hash of the previous block";
			return false;
		}

		if (block.dateTime < previousBlock->dateTime)
		{
			LOG << LogSeverity::error << "The current block is older than the previous block";
			return false;
		}


		const auto blockOfTheAuthorAtCreationTime = usergroupsManager->GetBlockInEffectAtTheTime(uuid, block.dateTime);
		if (block.permissions != previousBlock->permissions)
		{
			if (!blockOfTheAuthorAtCreationTime.HasPermission("CHANGE_PERMISSIONS"))
			{
				LOG << LogSeverity::error << "The permissions have changed since the previous block but the author of the block was not allowed to do this change.";
				return false;
			}
		}

		if (block.userList != previousBlock->userList)
		{
			if (!blockOfTheAuthorAtCreationTime.HasPermission("CHANGE_USERGROUP"))
			{
				LOG << LogSeverity::error << "The users list has changed since the previous block but the author of the block was not allowed to do this change.";
				return false;
			}
		}
	}

	return true;
}

void Usergroup::InsertNewBlock(const UsergroupBlock& block)
{
	m_blockchain.push_back(block);
	
	SQLQuery query(*global->database);
	query << INSERT_INTO << "usergroups_blockchain" << "(usergroup_uuid, version, block, mac, date_time)" <<
			 VALUES<< "(" << block.usergroupUuid.GetAsByteVector() << "," << static_cast<Int64>(block.version) << "," << block.blockRawBytes << "," << block.mac << "," << block.dateTime.str() << ")";
	query.ExecAsync();

	LOG << LogSeverity::verbose << "Added a new block to the usergroup " << uuid.GetAsByteVector() << " version: " << block.version;
}

bool Usergroup::ComparePreviousBlockHash(const UsergroupBlock& block, const UsergroupBlock* previousBlock)
{
	SECURITY_ASSERT(block.previousBlockHash.has_value());
	SECURITY_ASSERT(previousBlock->previousBlockHash.has_value());

	return (UsergroupBlock::HashBlock(*previousBlock) == block.previousBlockHash.value());
}

bool Usergroup::ValidateBlockMac(const UsergroupBlock& block, const PublicKeyBytes& signerPublicKey)
{
	return Cryptography::RSA::VerifySignature(block.mac, signerPublicKey, block.blockRawBytes);
}

bool Usergroup::CheckIfUserPresent(const UsergroupBlock& block, const Username& user)
{
	const auto it = std::find_if(block.userList.begin(), block.userList.end(), [&user](const Username& userInList)
	{
		return (userInList == user);
	});

	return (it != block.userList.end());
}

bool Usergroup::operator==(const Usergroup& other) const
{
	return (other.uuid == uuid);
}

Uuid Usergroup::GetUuid() const
{
	return uuid;
}

const UsergroupBlock& Usergroup::GetCurrentVersionBlock() const
{
	return m_blockchain.at(m_blockchain.size() - 1);
}
