#pragma once
#include "DateTimeUtils.h"
#include "Username.h"
#include "Uuid.h"
#include <optional>
#include "MessagePack.hpp"
#include "CryptographyTypes.hpp"
#include "IUsersManager.h"
class UsergroupsManager;

struct UsergroupBlock
{
	struct UserKey
	{
		Username username;
		EncryptedPrivateKeyBytes encryptedDecryptionKey;
	};

	Uuid usergroupUuid;
	UInt64 version = 0;
	PublicKeyBytes publicKey;
	IV iv;
	EncryptedPrivateKeyBytes encryptedPrivateKey;
	std::vector<Username> userList;
	std::vector<std::string> permissions;
	Username macSigner;
	std::vector<byte> mac;
	std::optional<hash_SHA3_256> previousBlockHash;
	DateTimeUtils::UTCDateTime dateTime;
	std::vector<UserKey> usersKeys;
	ByteContainer blockRawBytes;

	static std::optional<UsergroupBlock> CreateFromCommand(const MessagePackDeserializer& msgPack, const IUsersManager& usersManager/*, const Username& author*/);
	static std::optional<UsergroupBlock> CreateFromDatabase(const Uuid& usergroupUuid, UInt64 version, UsergroupsManager& usergrpManager);
	static bool CheckBlockMessagePackFields(const MessagePackDeserializer& blockMsgPack);
	static bool ReadRawBlock(const MessagePackDeserializer& blockMsgPack, UsergroupBlock& block, const IUsersManager& usersManager);

	bool HasPermission(const std::string& permission) const;
	void AddBlockToCmd(MessagePackSerializer& ser, const Username& author, const PrivateKeyBytes& authorIdKey, const IUsersManager& usersManager, UsergroupBlock* previousBlock = nullptr);

	static hash_SHA3_256 HashBlock(const UsergroupBlock& block);

private:
	static std::vector<Username> ReadUsernamesFromField(std::vector<ByteContainer> arr);
	static bool CheckPermissions(const UsergroupBlock& block);
	static bool CheckUsers(const UsergroupBlock& block, const IUsersManager& usergrpManager);
	static bool CheckUsersKeys(const UsergroupBlock& block);
	static std::vector<UserKey> ReadUserKeysFromCmd(const MessagePackDeserializer& msgPack);
};