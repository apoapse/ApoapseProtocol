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
		catch (const std::exception&)
		{
			std::cout << "Unable to load the dynamic library " << std::string(libraryName);
			std::abort();
		}

		return dll;
	}
};