#include "Common.h"

Global* global = nullptr;

void FatalError(const std::string& msg)
{
	LOG << LogSeverity::fatalError << msg;

	std::abort();
}