#pragma once
#include <map>
#include "Usergroup.h"
#include "Uuid.h"
#include "Username.h"
#include "IUsersManager.h"
#include "IUser.hpp"

class UsergroupsManager
{
	std::map<Uuid, Usergroup> m_usergroups; // #THREADING

public:
	const IUsersManager& usersManager;

	UsergroupsManager(IUsersManager& usersManager);
	//virtual ~UsergroupsManager();

	void Init();

	void InitializeUsergroup(const Uuid& groupUuid);
	static const std::vector<std::string>& GetAllowedPermissions();
	static bool IsPermissionExist(const std::string& permission);

	bool TryCommitNewBlockFromCommand(const MessagePackDeserializer& msgPack, IUser& author);

	bool DoesUsergroupExist(const Uuid& uuid) const;
	Usergroup& GetUsergroupByUuid(const Uuid& uuid);
	const Usergroup& GetUsergroupOfUser(const Username& username) const;

	UsergroupBlock GetBlockInEffectAtTheTime(const Uuid& usergroupUuid, const DateTimeUtils::UTCDateTime& dateTime);
	
private:
};