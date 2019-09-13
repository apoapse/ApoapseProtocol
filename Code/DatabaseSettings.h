#pragma once
#include "DataStructure.h"

class DatabaseSettings
{
	DataStructure m_settings;

public:
	DatabaseSettings() = default;
	DatabaseSettings(const std::string& name);

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
};
