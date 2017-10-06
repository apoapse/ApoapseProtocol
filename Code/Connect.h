#pragma once
#include "Command.h"

class CmdConnect : public Command
{
public:
	CommandInfo& GetInfo() const override;

private:
};