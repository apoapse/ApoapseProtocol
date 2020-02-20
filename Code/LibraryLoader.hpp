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
#include <boost/dll/import.hpp>
#include <iostream>
#include <boost/shared_ptr.hpp>

namespace LibraryLoader
{
	template <typename T>
	static boost::shared_ptr<T> LoadLibrary(const char* libraryName)
	{
		boost::shared_ptr<T> dll;
		try
		{
			dll = boost::dll::import<T>(libraryName, "apoapse_internal_dll", boost::dll::load_mode::append_decorations);
		}
		catch (const std::exception& e)
		{
			std::cout << "Unable to load the dynamic library " << std::string(libraryName) << " Exception: " << e.what();
			std::abort();
		}

		return dll;
	}
};