#pragma once
#include "TypeDefs.hpp"
#include "Username.h"
class CommandV2;

class ApoapseOperation
{
public:
	static void SaveOperation(CommandV2& cmd, const std::optional<Username>& sender);
};