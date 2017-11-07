#include "stdafx.h"
#include "UsergroupsManager.h"
#include "Common.h"
#include "SQLQuery.h"

UsergroupsManager::UsergroupsManager(IUsersManager& usersManager) : usersManager(usersManager)
{

}

void UsergroupsManager::Init()
{
	LOG << "Initializing usergoups...";

	try
	{
		SQLQuery query(*global->database);
		query << SELECT << "uuid" << FROM << "usergroups";
		auto res = query.Exec();
	
		if (!res)
			FatalError("Corrupted database");
	
		for (const auto& row : res)
		{
			const auto uuid = Uuid(row[0].GetByteArray());
			InitializeUsergroup(uuid);
		}
	}
	catch (const std::exception& e)
	{
		LOG << LogSeverity::error << e;
		FatalError("Unable to initialize the usergroups manager");
	}
}

void UsergroupsManager::InitializeUsergroup(const Uuid& groupUuid)
{
	auto group = Usergroup::CreateFromDatabase(groupUuid, *this);
}

const std::vector<std::string>& UsergroupsManager::GetAllowedPermissions()
{
	static std::vector<std::string> permissions
	{
		"CREATE_USER",
		"CHANGE_PERMISSIONS",
		"CREATE_USERGROUP",
		"CHANGE_USERGROUP"
	};

	return permissions;
}

bool UsergroupsManager::IsPermissionExist(const std::string& permission)
{
	const auto& allowedPerms = GetAllowedPermissions();

	return (std::find(allowedPerms.begin(), allowedPerms.end(), permission) != allowedPerms.end());
}

bool UsergroupsManager::TryCommitNewBlockFromCommand(const MessagePackDeserializer& msgPack, IUser& author)
{
	if (auto block = UsergroupBlock::CreateFromCommand(msgPack, *this))
	{
		if (!DoesUsergroupExist(block->usergroupUuid))
		{
			LOG << LogSeverity::error << "The usergroup specified on the block does not exist";
			return false;
		}

		Usergroup& usergroup = GetUsergroupByUuid(block->usergroupUuid);
		
		if (!usergroup.ValidateBlockInContext(block.value(), &usergroup.GetCurrentVersionBlock()))
			return false;

		if (block->macSigner != author.GetUsername())
		{
			LOG << LogSeverity::error << "Trying to insert a new username block from a user command but the mac signer is not the command user";
			return false;
		}

		usergroup.InsertNewBlock(block.value());
		// #TODO if server: send new block (avoir un système externe et universel de traitement des opérations?
		return true;
	}
	else
	{
		return false;
	}
}

bool UsergroupsManager::DoesUsergroupExist(const Uuid& uuid) const
{
	return (m_usergroups.count(uuid) != 0);
}

Usergroup& UsergroupsManager::GetUsergroupByUuid(const Uuid& uuid)
{
	return m_usergroups.at(uuid);
}

const Usergroup& UsergroupsManager::GetUsergroupOfUser(const Username& username) const
{
	for (const auto& usergroup : m_usergroups)
	{
		const auto& users = usergroup.second.GetCurrentVersionBlock().userList;
		
		auto res = std::find_if(users.begin(), users.end(), [&username](const Username& usernameInList)
		{
			return (usernameInList == username);
		});

		if (res != users.end())
			return usergroup.second;
	}

	throw std::exception("The usergroup for the provided username cannot be found");
}

size_t UsergroupsManager::GetUsegroupsCount() const
{
	return m_usergroups.size();
}

UsergroupBlock UsergroupsManager::GetBlockInEffectAtTheTime(const Uuid& usergroupUuid, const DateTimeUtils::UTCDateTime& dateTime)
{
	SQLQuery query(*global->database);
	query << SELECT << "version" << FROM << "usergroups_blockchain" << WHERE << "usergroup_uuid" << EQUALS << usergroupUuid.GetAsByteVector()
		  << AND << "date_time" << " < " << dateTime.str() << ORDER_BY << "date_time" << ASC << LIMIT << "1";
	auto res = query.Exec();

	if (res && res.RowCount() == 1)
	{
		const Int64 version = res[0][0].GetInt64();
		return UsergroupBlock::CreateFromDatabase(usergroupUuid, version, *this).value();
	}
	else
	{
		throw std::exception("There was no usergroup block in effect at the specified time");
	}
}
