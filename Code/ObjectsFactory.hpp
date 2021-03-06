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
#include <map>
#include <memory>
#include "Diagnostic.hpp"

template <typename KEY, typename T_BASE_CLASS>
class ObjectsFactory
{
	struct IObjectCreator
	{
		virtual ~IObjectCreator() = default;
		virtual std::unique_ptr<T_BASE_CLASS> Create() = 0;
	};

	template <typename T_CLASS>
	class Creator : public IObjectCreator
	{
	public:
		std::unique_ptr<T_BASE_CLASS> Create()
		{
			return std::make_unique<T_CLASS>();
		}
	};

	std::map<KEY, std::unique_ptr<IObjectCreator>> m_objectFactories;

public:
	bool DoesObjectExist(const KEY& index) const
	{
		return (m_objectFactories.count(index) > 0);
	}

	std::unique_ptr<T_BASE_CLASS> CreateObject(const KEY& index)
	{
		ASSERT_MSG(DoesObjectExist(index), "The requested object does not exist");

		return m_objectFactories[index]->Create();
	}

	template <typename T_CLASS>
	void RegisterObject(const KEY& index)
	{
		static_assert(std::is_base_of<T_BASE_CLASS, T_CLASS>::value);
		ASSERT_MSG(!DoesObjectExist(index), "This object is already registered");

		m_objectFactories[index] = std::make_unique<Creator<T_CLASS>>();
	}
};

template <typename T_BASE_CLASS, typename T_CLASS, typename T_INDEX>
class ObjectAutoRegister
{
public:
	ObjectAutoRegister(ObjectsFactory<T_INDEX, T_BASE_CLASS>& objectsFactory, const T_INDEX& index)
	{
		objectsFactory.template RegisterObject<T_CLASS>(index);
	}
};
#define OBJECTS_FACTORY_REGISTER(_factoryInstance, _baseClass, _class, _indexType, _index)	static ObjectAutoRegister<_baseClass, _class, _indexType> objRegistration(_factoryInstance, _index);
#define OBJECTS_FACTORY_REGISTER_2(_factoryInstance, _baseClass, _class, _indexType, _index)	static ObjectAutoRegister<_baseClass, _class, _indexType> objRegistration2(_factoryInstance, _index);