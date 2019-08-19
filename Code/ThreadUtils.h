#pragma once
#include <string>
#include <map>

namespace ThreadUtils
{
	inline std::map<std::thread::id, std::string> threadNames = {};
	
	void NameThread(std::string name);
	std::string GetThreadName();
	std::string GetThreadName(std::thread::id& threadId);
}