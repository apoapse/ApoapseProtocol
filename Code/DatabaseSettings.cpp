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
