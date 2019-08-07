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

	void CreateUsergroup(const Uuid& uuid, const std::string& name, const std::string& permissions);
};