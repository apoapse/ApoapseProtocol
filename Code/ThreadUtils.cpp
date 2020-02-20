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
#include "ThreadUtils.h"
#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
const DWORD MS_VC_EXCEPTION = 0x406D1388;  
typedef struct tagTHREADNAME_INFO  
{  
    DWORD dwType; // Must be 0x1000.  
    LPCSTR szName; // Pointer to name (in user addr space).  
    DWORD dwThreadID; // Thread ID (-1=caller thread).  
    DWORD dwFlags; // Reserved for future use, must be zero.  
 } THREADNAME_INFO;  

void WINSetThreadName(DWORD dwThreadID, const char* threadName) {  
    THREADNAME_INFO info;  
    info.dwType = 0x1000;  
    info.szName = threadName;  
    info.dwThreadID = dwThreadID;  
    info.dwFlags = 0;  
#pragma warning(push)  
#pragma warning(disable: 6320 6322)  
    __try{  
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);  
    }  
    __except (EXCEPTION_EXECUTE_HANDLER){  
    }  
#pragma warning(pop)  
}
#endif

void ThreadUtils::NameThread(std::string name)
{
	threadNames[std::this_thread::get_id()] = name;
		
	#ifdef _WIN32
	WINSetThreadName(GetCurrentThreadId(), name.c_str());
	#endif
}

std::string ThreadUtils::GetThreadName()
{
	auto currentThreadId = std::this_thread::get_id();

	return GetThreadName(currentThreadId);
}

std::string ThreadUtils::GetThreadName(std::thread::id& threadId)
{
	std::stringstream ss;

	if (threadNames.count(threadId))
		ss << threadNames.at(threadId);
	else
		ss << threadId;
	
	return ss.str();
}
