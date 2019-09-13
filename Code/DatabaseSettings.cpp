#include "stdafx.h"
#include "Common.h"
#include "DatabaseSettings.h"

DatabaseSettings::DatabaseSettings(const std::string& name)
{
	m_settings = global->apoapseData->ReadItemFromDatabase("server_setting", "id", 1);
}
