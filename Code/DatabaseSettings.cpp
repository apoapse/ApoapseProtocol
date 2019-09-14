#include "stdafx.h"
#include "Common.h"
#include "DatabaseSettings.h"

DatabaseSettings::DatabaseSettings(const std::string& name)
{
	m_settings = global->apoapseData->ReadItemFromDatabase(name, "id", 1);
}

DatabaseSettings::DatabaseSettings(const DataStructure& dat)
{
	m_settings = dat;
}

DataStructure DatabaseSettings::GetDataStructure() const
{
	return m_settings;
}
