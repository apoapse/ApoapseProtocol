// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#include "stdafx.h"
#include "Common.h"
#include "UsergroupManager.h"
#include "StringExtensions.h"

Usergroup::Usergroup(DataStructure& data)
{
	m_uuid = data.GetField("uuid").GetValue<Uuid>();
	m_name = data.GetField("name").GetValue<std::string>();

	if (data.GetField("permissions").HasValue())
	{
		StringExtensions::split(data.GetField("permissions").GetValue<std::string>(), m_permissions, " ");
	}
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
	const auto res = std::find(m_permissions.begin(), m_permissions.end(), permName);

	return (res != m_permissions.end());
}

const std::vector<std::string>& Usergroup::GetPermissions() const
{
	return m_permissions;
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
	const auto res = std::find(m_registeredUsergroups.begin(), m_registeredUsergroups.end(), uuid);

	if (res == m_registeredUsergroups.end())
		throw std::runtime_error("No usergroup with the provided uuid");

	return *res;
}

const Usergroup& UsergroupManager::GetUsergroup(const std::string& name) const
{
	const auto res = std::find(m_registeredUsergroups.begin(), m_registeredUsergroups.end(), name);

	if (res == m_registeredUsergroups.end())
		throw std::runtime_error("No usergroup with the provided name");

	return *res;
}

const std::vector<Usergroup>& UsergroupManager::GetUsergroups() const
{
	return m_registeredUsergroups;
}

void UsergroupManager::CreateUsergroup(const Uuid& uuid, const std::string& name, const std::string& permissions)
{
	auto dat = global->apoapseData->GetStructure("usergroup");
	dat.GetField("uuid").SetValue(uuid);
	dat.GetField("name").SetValue(name);
	dat.GetField("permissions").SetValue(permissions);

	dat.SaveToDatabase();

	auto usergroup = Usergroup(dat);
	RegisterUsergroup(usergroup);
}
