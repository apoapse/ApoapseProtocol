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

#pragma once
#include "TypeDefs.hpp"
#include "Uuid.h"
class DataStructure;

class Usergroup
{
	Uuid m_uuid;
	std::string m_name;
	std::vector<std::string> m_permissions = {};

public:
	Usergroup(DataStructure& data);
	bool operator==(const Uuid& uuid) const;
	bool operator==(const std::string& name) const;

	Uuid GetUuid() const;
	std::string GetName() const;

	bool HasPermission(const std::string& permName) const;
	const std::vector<std::string>& GetPermissions() const;
};

class UsergroupManager
{
	std::vector<Usergroup> m_registeredUsergroups = {};

public:
	UsergroupManager();
	UsergroupManager(std::vector<DataStructure>& usergroupsDat);

	void RegisterUsergroup(Usergroup& usergroup);
	const Usergroup& GetUsergroup(const Uuid& uuid) const;
	const Usergroup& GetUsergroup(const std::string& name) const;
	const std::vector<Usergroup>& GetUsergroups() const;

	void CreateUsergroup(const Uuid& uuid, const std::string& name, const std::string& permissions);
};