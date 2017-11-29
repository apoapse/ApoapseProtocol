#pragma once
#include "TypeDefs.hpp"
#include "CryptographyTypes.hpp"
#include "Username.h"
#include "Uuid.h"
#include "MessagePack.hpp"
#include <optional>
#include "UsergroupBlock.h"
class UsergroupsManager;

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

	Uuid GetUuid() const;
	const UsergroupBlock& GetCurrentVersionBlock() const;

	bool ValidateBlockInContext(const UsergroupBlock& block, const UsergroupBlock* previousBlock);
	void InsertNewBlock(const UsergroupBlock& block);

private:
	static bool ComparePreviousBlockHash(const UsergroupBlock& block, const UsergroupBlock* previousBlock);
	static bool ValidateBlockMac(const UsergroupBlock& block, const PublicKeyBytes& signerPublicKey);
	static bool CheckIfUserPresent(const UsergroupBlock& block, const Username& user);
};