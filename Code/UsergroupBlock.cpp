#include "stdafx.h"
#include "UsergroupBlock.h"
#include "Common.h"
#include "RSA.hpp"
#include "Hash.hpp"
#include "AES.hpp"
#include "SQLQuery.h"
#include "UsergroupsManager.h"

std::optional<UsergroupBlock> UsergroupBlock::CreateFromCommand(const MessagePackDeserializer& msgPack, const IUsersManager& usersManager/*, const Username& author*/)
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
			if (ReadRawBlock(*blockMsgPack, block, usersManager))
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

		if (!ReadRawBlock(blockMsgPack, block, usergrpManager.usersManager))
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
		|| !blockMsgPack.GetValueOptional<ByteContainer>("private_key.iv")
		|| !blockMsgPack.GetValueOptional<ByteContainer>("private_key.encrypted")
		|| !blockMsgPack.GetArrayOptional<ByteContainer>("users")
		|| !blockMsgPack.GetArrayOptional<std::string>("permissions")
		|| !blockMsgPack.GetValueOptional<std::string>("mac_algorithm")
		|| !blockMsgPack.GetValueOptional<ByteContainer>("mac_user_signer")
		//|| !blockMsgPack.GetValueOptional<std::string>("previous_block.hash_algorithm")
		)
	{
		return false;
	}

	if (blockMsgPack.GetValue<UInt64>("version") < 1)
		return false;

	if (!DateTimeUtils::UTCDateTime::ValidateFormat(blockMsgPack.GetValue<std::string>("date_time")))
		return false;

	if (blockMsgPack.GetValue<std::string>("keypair_algorithm") != "RSA2048")
		return false;

	if (blockMsgPack.GetValue<std::string>("private_key.algorithm") != "AES256_CBC")
		return false;

	if (blockMsgPack.GetValue<std::string>("mac_algorithm") != "RSA SHA256")
		return false;

	/*if (blockMsgPack.GetValue<std::string>("previous_block.hash_algorithm") != "SHA3 256")
		return false;*/

	if (!Username::IsValid(blockMsgPack.GetValue<ByteContainer>("mac_user_signer")))
		return false;

	if (blockMsgPack.GetValue<UInt64>("version") > 1)
	{
		if (blockMsgPack.GetValue<ByteContainer>("previous_block.hash").size() != sha256Length)
			return false;
	}

	return true;
}

bool UsergroupBlock::ReadRawBlock(const MessagePackDeserializer& blockMsgPack, UsergroupBlock& block, const IUsersManager& usersManager)
{
	if (!CheckBlockMessagePackFields(blockMsgPack))
		return false;

	block.version = blockMsgPack.GetValue<UInt64>("version");
	block.dateTime = DateTimeUtils::UTCDateTime(blockMsgPack.GetValue<std::string>("date_time"));
	block.publicKey = blockMsgPack.GetValue<ByteContainer>("public_key");
	block.iv = VectorToArray<byte, 16>(blockMsgPack.GetValue<ByteContainer>("private_key.iv"));
	block.encryptedPrivateKey = blockMsgPack.GetValue<ByteContainer>("private_key.encrypted");
	block.userList = ReadUsernamesFromField(blockMsgPack.GetArray<ByteContainer>("users"));
	block.permissions = blockMsgPack.GetArray<std::string>("permissions");
	block.macSigner = Username(blockMsgPack.GetValue<ByteContainer>("mac_user_signer"));
	block.usersKeys = ReadUserKeysFromCmd(blockMsgPack);

	if (block.version > 1)
		block.previousBlockHash = VectorToArray<byte, sha256Length>(blockMsgPack.GetValue<ByteContainer>("previous_block.hash"));

	if (block.iv.empty() || block.encryptedPrivateKey.empty())
		return false;

	if (block.dateTime > DateTimeUtils::UTCDateTime::CurrentTime())
	{
		LOG << LogSeverity::error << "The block datetime is greater than the current time (on this machine)";
		return false;
	}

	if (!block.previousBlockHash.has_value() && block.version > 1)
	{
		LOG << LogSeverity::error << "The block is not the first of a usergroup but does not have a hash of the previous block";
		return false;
	}

	if (!CheckPermissions(block) || !CheckUsers(block, usersManager) || !CheckUsersKeys(block))
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

bool UsergroupBlock::CheckUsers(const UsergroupBlock& block, const IUsersManager& usergrpManager)
{
	// Check the user list
	for (const auto& username : block.userList)
	{
		if (!usergrpManager.DoesUserExist(username))
		{
			LOG << LogSeverity::error << "One of the users of the usergroup block cannot be found on this server";
			return false;
		}
	}

	// Check the signer user
	if (!usergrpManager.DoesUserExist(block.macSigner))
	{
		LOG << LogSeverity::error << "The mac signer username cannot be found on the database";
		return false;
	}

	return true;
}

bool UsergroupBlock::CheckUsersKeys(const UsergroupBlock& block)
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
		const auto res = std::find_if(block.usersKeys.begin(), block.usersKeys.end(), [&username](const UserKey& userKeys)
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

std::vector<UsergroupBlock::UserKey> UsergroupBlock::ReadUserKeysFromCmd(const MessagePackDeserializer& msgPack)
{
	std::vector<UsergroupBlock::UserKey> output;

	if (auto fields = msgPack.GetArrayOptional<MessagePackDeserializer>("users_keys"))
	{
		for (const auto& item : *fields)
		{
			if (!item.GetValueOptional<ByteContainer>("username") || !item.GetValueOptional<ByteContainer>("decryption_key"))
				return std::vector<UsergroupBlock::UserKey>();

			if (!Username::IsValid(item.GetValue<ByteContainer>("username")))
				return std::vector<UsergroupBlock::UserKey>();

			if (item.GetValue<ByteContainer>("decryption_key").empty())
				return std::vector<UsergroupBlock::UserKey>();

			UsergroupBlock::UserKey keyDef;
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

void UsergroupBlock::AddBlockToCmd(MessagePackSerializer& ser, const Username& author, const PrivateKeyBytes& authorIdKey, const IUsersManager& usersManager, UsergroupBlock* previousBlock /*= nullptr*/)
{
	if (previousBlock && previousBlock->version != version - 1)
		throw std::exception("The block version is not equal to the previous block version +1");

	PublicKeyBytes blockMac;
	const auto[privateKey, publicKey] = Cryptography::RSA::GenerateKeyPair(2048);
	const auto[aesPrivateKey, iv, encryptedPrivateKey] = Cryptography::AES_CBC::Encrypt(std::vector<byte>(privateKey.begin(), privateKey.end()), 32);

	ser.UnorderedAppend("usergroup_block.uuid", usergroupUuid.GetAsByteVector());

	{
		std::vector<ByteContainer> users;
		for (const auto& username : userList)
		{
			const auto usernameRawBytes = username.GetRaw();
			users.emplace_back(usernameRawBytes.begin(), usernameRawBytes.end());
		}

		MessagePackSerializer serBlock;
		serBlock.UnorderedAppend("version", version);
		serBlock.UnorderedAppend("date_time", DateTimeUtils::UTCDateTime::CurrentTime().str());
		serBlock.UnorderedAppend("keypair_algorithm", "RSA2048"s);
		serBlock.UnorderedAppend("public_key", publicKey);
		serBlock.UnorderedAppend("private_key.algorithm", "AES256_CBC"s);
		serBlock.UnorderedAppend("private_key.iv", iv);
		serBlock.UnorderedAppend("private_key.encrypted", encryptedPrivateKey);
		serBlock.UnorderedAppendArray<ByteContainer>("users", users);
		serBlock.UnorderedAppendArray<std::string>("permissions", permissions);
		serBlock.UnorderedAppend("mac_algorithm", "RSA SHA256"s);
		serBlock.UnorderedAppend("mac_user_signer", author.GetRaw());

		if (previousBlock)
		{
			serBlock.UnorderedAppend("previous_block.hash_algorithm", "SHA3 256"s);
			serBlock.UnorderedAppend("previous_block.hash", HashBlock(*previousBlock));
		}

		{
			std::vector<MessagePackSerializer> usersKey;

			for (const Username& user : userList)
			{
				const auto userPublicKey = usersManager.GetUserIdentityPublicKey(user);

				MessagePackSerializer serUser;
				serUser.UnorderedAppend("username", user.GetRaw());
				serUser.UnorderedAppend("decryption_key", Cryptography::RSA::Encrypt(userPublicKey, std::vector<byte>(aesPrivateKey.begin(), aesPrivateKey.end())));

				usersKey.push_back(serUser);
			}

			serBlock.UnorderedAppendArray("users_keys", usersKey);
		}

		blockMac = Cryptography::RSA::Sign(authorIdKey, serBlock.GetMessagePackBytes());
		ser.UnorderedAppend("usergroup_block.block", serBlock);
	}

	ser.UnorderedAppend("usergroup_block.mac", blockMac);
}

hash_SHA3_256 UsergroupBlock::HashBlock(const UsergroupBlock& block)
{
	ASSERT(!block.blockRawBytes.empty());
	return Cryptography::SHA3_256(block.blockRawBytes);
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
