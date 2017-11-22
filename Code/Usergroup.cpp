#include "stdafx.h"
#include "RSA.hpp"
#include "Usergroup.h"
#include "Common.h"
#include "UsergroupsManager.h"
#include "MemoryUtils.hpp"
#include "SQLQuery.h"
#include "Hash.hpp"
#include "UsergroupsManager.h"

std::optional<UsergroupBlock> UsergroupBlock::CreateFromCommand(const MessagePackDeserializer& msgPack, UsergroupsManager& usergrpManager/*, const Username& author*/)
{
	try
	{
		UsergroupBlock block{};
	
		{
			if (!msgPack.GetValueOptional<ByteContainer>("usergroup_block.uuid") || !Uuid::IsValid(msgPack.GetValue<ByteContainer>("usergroup_block.uuid")))
				return std::optional<UsergroupBlock>();

			block.usergroupUuid = Uuid(msgPack.GetValue<ByteContainer>("usergroup_block.uuid"));
		}
	
		{
			if (!msgPack.GetValueOptional<ByteContainer>("usergroup_block.mac")/* || msgPack.GetValue<ByteContainer>("usergroup_block.mac").size() != 256*/)
				return std::optional<UsergroupBlock>();
	
			block.mac = msgPack.GetValue<ByteContainer>("usergroup_block.mac");
		}
	
		if (const auto blockMsgPack = msgPack.GetValueOptional<MessagePackDeserializer>("usergroup_block.block"))
		{
			if (ReadRawBlock(*blockMsgPack, block, usergrpManager))
				block.blockRawBytes = msgPack.GetValue<ByteContainer>("usergroup_block.block");
			else
				return std::optional<UsergroupBlock>();
		}
		else
		{
			return std::optional<UsergroupBlock>();
		}
	
		return block;
	}
	catch (const std::exception&)
	{
		return std::optional<UsergroupBlock>();
	}
}

std::optional<UsergroupBlock> UsergroupBlock::CreateFromDatabase(const Uuid& usergroupUuid, UInt64 version, UsergroupsManager& usergrpManager)
{
	SQLQuery query(*global->database);
	query << SELECT << "block, MAC, date_time" << FROM << "usergroups_blockchain" << WHERE << "usergroupUuid" << EQUALS << usergroupUuid.GetAsByteVector()
		  << AND << "version" << EQUALS << (Int64)version;
	auto res = query.Exec();

	if (res && res.RowCount() == 1)
	{
		UsergroupBlock block{};
		block.version = version;
		block.blockRawBytes = res[0][0].GetByteArray();
		block.mac = res[0][1].GetByteArray();
		block.dateTime = res[0][2].GetText();

		const MessagePackDeserializer blockMsgPack(block.blockRawBytes);

		if (!ReadRawBlock(blockMsgPack, block, usergrpManager))
			return std::optional<UsergroupBlock>();

		return block;
	}
	else
	{
		return std::optional<UsergroupBlock>();
	}
}

bool UsergroupBlock::CheckBlockMessagePackFields(const MessagePackDeserializer& blockMsgPack)
{
	if (!blockMsgPack.GetValueOptional<UInt64>("version")
		|| !blockMsgPack.GetValueOptional<std::string>("date_time")
		|| !blockMsgPack.GetValueOptional<std::string>("keypair_algorithm")
		|| !blockMsgPack.GetValueOptional<ByteContainer>("public_key")
		|| !blockMsgPack.GetValueOptional<std::string>("private_key.algorithm")
		|| !blockMsgPack.GetValueOptional<ByteContainer>("private_key.encrypted")
		|| !blockMsgPack.GetArrayOptional<ByteContainer>("users")
		|| !blockMsgPack.GetArrayOptional<std::string>("permissions")
		|| !blockMsgPack.GetValueOptional<std::string>("mac_algorithm")
		|| !blockMsgPack.GetValueOptional<ByteContainer>("mac_user_signer")
		|| !blockMsgPack.GetValueOptional<std::string>("previous_block.hash_algorithm")
		|| !blockMsgPack.GetValueOptional<ByteContainer>("previous_block.hash"))
	{
		return false;
	}

	if (!DateTimeUtils::UTCDateTime::ValidateFormat(blockMsgPack.GetValue<std::string>("date_time")))
		return false;

	if (blockMsgPack.GetValue<std::string>("keypair_algorithm") != "RSA2048")
		return false;

	if (blockMsgPack.GetValue<std::string>("private_key.algorithm") != "AES256")
		return false;

	if (blockMsgPack.GetValue<std::string>("mac_algorithm") != "RSA SHA256")
		return false;

	if (blockMsgPack.GetValue<std::string>("previous_block.hash_algorithm") != "SHA3 256")
		return false;

	if (!Username::IsValid(blockMsgPack.GetValue<ByteContainer>("mac_user_signer")))
		return false;

	if (blockMsgPack.GetValue<ByteContainer>("previous_block.hash").size() != sha256Length)
		return false;

	return true;
}

bool UsergroupBlock::ReadRawBlock(const MessagePackDeserializer& blockMsgPack, UsergroupBlock& block, UsergroupsManager& usergrpManager)
{
	if (!CheckBlockMessagePackFields(blockMsgPack))
		return false;

	block.version = blockMsgPack.GetValue<UInt64>("version");
	block.dateTime = DateTimeUtils::UTCDateTime(blockMsgPack.GetValue<std::string>("date_time"));
	block.publicKey = blockMsgPack.GetValue<ByteContainer>("public_key");
	block.encryptedPrivateKey = blockMsgPack.GetValue<ByteContainer>("private_key.encrypted");
	block.userList = ReadUsernamesFromField(blockMsgPack.GetArray<ByteContainer>("users"));
	block.permissions = blockMsgPack.GetArray<std::string>("permissions");
	block.previousBlockHash = VectorToArray<byte, sha256Length>(blockMsgPack.GetValue<ByteContainer>("previous_block.hash"));
	block.macSigner = Username(blockMsgPack.GetValue<ByteContainer>("mac_user_signer"));
	block.usersKeys = ReadUserKeysFromCmd(blockMsgPack);

	if (block.dateTime > DateTimeUtils::UTCDateTime::CurrentTime())
	{
		LOG << LogSeverity::error << "The block datetime is greater than the current time (on this machine)";
		return false;
	}

	if (!CheckPermissions(block) || !CheckUsers(block, usergrpManager) || !CheckUsersKeys(block, usergrpManager))
		return false;

	return true;
}

bool UsergroupBlock::CheckPermissions(const UsergroupBlock& block)
{
	for (const auto& perm : block.permissions)
	{
		if (!UsergroupsManager::DoesPermissionExist(perm))
		{
			LOG << LogSeverity::error << "One of the permissions of the usergroup block is not present in the whitelist";
			return false;
		}
	}

	return true;
}

bool UsergroupBlock::CheckUsers(const UsergroupBlock& block, UsergroupsManager& usergrpManager)
{
	// Check the user list
	for (const auto& username : block.userList)
	{
		if (!usergrpManager.usersManager.DoesUserExist(username))
		{
			LOG << LogSeverity::error << "One of the users of the usergroup block cannot be found on this server";
			return false;
		}
	}

	// Check the signer user
	if (!usergrpManager.usersManager.DoesUserExist(block.macSigner))
	{
		LOG << LogSeverity::error << "The mac signer username cannot be found on the database";
		return false;
	}

	return true;
}

bool UsergroupBlock::CheckUsersKeys(const UsergroupBlock& block, UsergroupsManager& usergrpManager)
{
	if (block.usersKeys.empty())
	{
		LOG << LogSeverity::error << "The users keys list is empty";
		return false;
	}

	if (block.usersKeys.size() != block.userList.size())
	{
		LOG << LogSeverity::error << "The number of users keys does not match the number of users defined in the usergroup block";
		return false;
	}

	for (const auto& username : block.userList)
	{
		const auto res = std::find_if(block.usersKeys.begin(), block.usersKeys.end(), [&username](const UserKeys& userKeys)
		{
			return (userKeys.username == username);
		});

		if (res == block.usersKeys.end())
		{
			LOG << LogSeverity::error << "One of the users of the block does not have a corresponding encrypted private key";
			return false;
		}
	}

	return true;
}

std::vector<UsergroupBlock::UserKeys> UsergroupBlock::ReadUserKeysFromCmd(const MessagePackDeserializer& msgPack)
{
	std::vector<UsergroupBlock::UserKeys> output;

	if (auto fields = msgPack.GetArrayOptional<MessagePackDeserializer>("users_keys"))
	{
		for (const auto& item : *fields)
		{
			if (!item.GetValueOptional<ByteContainer>("username") || !item.GetValueOptional<ByteContainer>("decryption_key"))
				return std::vector<UsergroupBlock::UserKeys>();

			if (!Username::IsValid(item.GetValue<ByteContainer>("username")))
				return std::vector<UsergroupBlock::UserKeys>();

			if (item.GetValue<ByteContainer>("decryption_key").empty())
				return std::vector<UsergroupBlock::UserKeys>();

			UsergroupBlock::UserKeys keyDef;
			keyDef.username = Username(item.GetValue<ByteContainer>("username"));
			keyDef.encryptedDecryptionKey = item.GetValue<ByteContainer>("decryption_key");

			output.push_back(keyDef);
		}
	}

	return output;
}

bool UsergroupBlock::HasPermission(const std::string& perm) const
{
	ASSERT_MSG(UsergroupsManager::DoesPermissionExist(perm), "Trying to compare a permission that does not exist.");

	return (std::find(permissions.begin(), permissions.end(), perm) != permissions.end());
}

std::vector<Username> UsergroupBlock::ReadUsernamesFromField(std::vector<ByteContainer> arr)
{
	std::vector<Username> usernamesList;

	for (const auto& userHash : arr)
	{
		if (!Username::IsValid(userHash))
			throw std::exception("Username invalid");

		usernamesList.emplace_back(userHash);
	}

	return usernamesList;
}


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
			if (!UsergroupBlock::ReadRawBlock(deserializer, block, *usergroupsManager))
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
	return (Cryptography::SHA3_256(previousBlock->blockRawBytes) == block.previousBlockHash);
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

const UsergroupBlock& Usergroup::GetCurrentVersionBlock() const
{
	return m_blockchain.at(m_blockchain.size() - 1);
}
