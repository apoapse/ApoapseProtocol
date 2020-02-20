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
#include "DataStructure.h"

class DatabaseSettings
{
	DataStructure m_settings;

public:
	DatabaseSettings() = default;
	DatabaseSettings(const std::string& name);
	DatabaseSettings(const DataStructure& dat);

	template <typename T>
	void SetValue(const std::string& fieldName, const T& value)
	{
		auto& field = m_settings.GetField(fieldName);
		
		field.SetValue(value);
		field.SaveToDatabase(m_settings, 1);
	}

	template <typename T>
	T GetValue(const std::string& name)
	{
		return m_settings.GetField(name).GetValue<T>();
	}

	template <typename T>
	T GetValueOr(const std::string& name, const T& optional)
	{
		return m_settings.GetField(name).GetValueOr<T>(optional);
	}

	DataStructure GetDataStructure() const;
};
