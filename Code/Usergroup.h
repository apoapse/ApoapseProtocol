#pragma once
#include "TypeDefs.hpp"
#include "CryptographyTypes.hpp"
#include "Username.h"
#include "Uuid.h"
#include "MessagePack.hpp"
#include <optional>
#include "DateTimeUtils.h"
class UsergroupsManager;

struct UsergroupBlock
{
	struct UserKeys
	{
		Username username;
		EncryptedPrivateKeyBytes encryptedDecryptionKey;
	};

	Uuid usergroupUuid;
	UInt64 version = 0;
	PublicKeyBytes publicKey;
	EncryptedPrivateKeyBytes encryptedPrivateKey;
	std::vector<Username> userList;
	std::vector<std::string> permissions;
	Username macSigner;
	std::vector<byte> mac;
	hash_SHA3_256 previousBlockHash;
	DateTimeUtils::UTCDateTime dateTime;
	std::vector<UserKeys> usersKeys;
	ByteContainer blockRawBytes;

	static std::optional<UsergroupBlock> CreateFromCommand(const MessagePackDeserializer& msgPack, UsergroupsManager& usergrpManager/*, const Username& author*/);
	static std::optional<UsergroupBlock> CreateFromDatabase(const Uuid& usergroupUuid, UInt64 version, UsergroupsManager& usergrpManager);
	static bool CheckBlockMessagePackFields(const MessagePackDeserializer& blockMsgPack);
	static bool ReadRawBlock(const MessagePackDeserializer& blockMsgPack, UsergroupBlock& block, UsergroupsManager& usergrpManager);

	bool HasPermission(const std::string& permission) const;

private:
	static std::vector<Username> ReadUsernamesFromField(std::vector<ByteContainer> arr);
	static bool CheckPermissions(const UsergroupBlock& block);
	static bool CheckUsers(const UsergroupBlock& block, UsergroupsManager& usergrpManager);
	static bool CheckUsersKeys(const UsergroupBlock& block, UsergroupsManager& usergrpManager);
	static std::vector<UserKeys> ReadUserKeysFromCmd(const MessagePackDeserializer& msgPack);
};

class Usergroup
{
	UsergroupsManager* usergroupsManager;
	std::vector<UsergroupBlock> m_blockchain;	// We expect this list to be sorted from the first version to the last #THREADING

public:
	size_t currentVersion = 0;
	Uuid uuid;

	Usergroup(const Uuid& uuid, UsergroupsManager* usergrpManager);
	Usergroup() = default;

	static Usergroup CreateFromDatabase(const Uuid& uuid, UsergroupsManager& usergrpManager);
	void ConstructFromDatabase();

	bool operator==(const Usergroup& other) const;

	const UsergroupBlock& GetCurrentVersionBlock() const;
	bool ValidateBlockInContext(const UsergroupBlock& block, const UsergroupBlock* previousBlock);
	void InsertNewBlock(const UsergroupBlock& block);

private:
	static bool ComparePreviousBlockHash(const UsergroupBlock& block, const UsergroupBlock* previousBlock);
	static bool ValidateBlockMac(const UsergroupBlock& block, const PublicKeyBytes& signerPublicKey);
	static bool CheckIfUserPresent(const UsergroupBlock& block, const Username& user);
};