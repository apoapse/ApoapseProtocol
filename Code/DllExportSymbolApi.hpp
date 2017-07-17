#pragma once

#ifdef DYN_LIBRARY  
#define DLL_EXPOSE_SYMBOL __declspec(dllexport)
#else  
#define DLL_EXPOSE_SYMBOL __declspec(dllimport)
#endif  