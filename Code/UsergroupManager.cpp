#include "stdafx.h"
#include "Common.h"
#include "UsergroupManager.h"
#include "StringExtensions.h"

Usergroup::Usergroup(DataStructure& data)
{
	m_uuid = data.GetField("uuid").GetValue<Uuid>();
	m_name = data.GetField("name").GetValue<std::string>();

	StringExtensions::split(data.GetField("permissions").GetValue<std::string>(), m_permissions, " ");
}

bool Usergroup::operator==(const Uuid& uuid) const
{
	return (GetUuid() == uuid);
}

bool Usergroup::operator==(const std::string& name) const
{
	return (GetName() == name);
}

Uuid Usergroup::GetUuid() const
{
	return m_uuid;
}

std::string Usergroup::GetName() const
{
	return m_name;
}

bool Usergroup::HasPermission(const std::string& permName) const
{
	auto res = std::find(m_permissions.begin(), m_permissions.end(), permName);

	return (res != m_permissions.end());
}

UsergroupManager::UsergroupManager()
{
	ASSERT(global->isServer);

	auto usergroupsDat = global->apoapseData->ReadListFromDatabase("usergroup", "", "");
	for (auto& dat : usergroupsDat)
	{
		m_registeredUsergroups.push_back(Usergroup(dat));
	}

	LOG << "Loaded " << m_registeredUsergroups.size() << " usergroups";
}

UsergroupManager::UsergroupManager(std::vector<DataStructure>& usergroupsDat)
{
	for (auto& dat : usergroupsDat)
	{
		m_registeredUsergroups.push_back(Usergroup(dat));
	}

	LOG << "Loaded " << m_registeredUsergroups.size() << " usergroups";
}

void UsergroupManager::RegisterUsergroup(Usergroup& usergroup)
{
	m_registeredUsergroups.push_back(usergroup);

	LOG << "Registered new usergroup " << usergroup.GetName();
}

const Usergroup& UsergroupManager::GetUsergroup(const Uuid& uuid) const
{
	auto res = std::find(m_registeredUsergroups.begin(), m_registeredUsergroups.end(), uuid);

	if (res == m_registeredUsergroups.end())
		throw std::exception("No usergroup with the provided uuid");

	return *res;
}

const Usergroup& UsergroupManager::GetUsergroup(const std::string& name) const
{
	auto res = std::find(m_registeredUsergroups.begin(), m_registeredUsergroups.end(), name);

	if (res == m_registeredUsergroups.end())
		throw std::exception("No usergroup with the provided name");

	return *res;
}

void UsergroupManager::CreateUsergroup(const Uuid& uuid, const std::string& name, const std::string& permissions)
{
	auto dat = global->apoapseData->GetStructure("usergroup");
	dat.GetField("uuid").SetValue(uuid);
	dat.GetField("name").SetValue(name);
	dat.GetField("permissions").SetValue(permissions);

	dat.SaveToDatabase();

	RegisterUsergroup(Usergroup(dat));
}

