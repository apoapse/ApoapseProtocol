#include "stdafx.h"

#ifdef UNIT_TESTS
#include "Common.h"
#include "Uuid.h"
#include "CommandsManager.h"

class CmdUnitTest : public Command
{
public:
	CommandInfo& GetInfo() const override
	{
		static auto info = CommandInfo();
		info.command = CommandId::unit_test_cmd;
		info.fields =
		{
			CommandField{ "optional_field", FieldRequirement::any_optional, FIELD_VALUE(UInt16) },
			CommandField{ "mendatory_field_uuid_validator", FieldRequirement::any_mendatory, FIELD_VALUE_VALIDATOR(std::vector<byte>, Uuid::IsValid) },
		};

		return info;
	}

// 	static bool Test(UInt16 val)
// 	{
// 		LOG << "TEST " << val;
// 		return (val < 10);
// 	}
	
private:
};

APOAPSE_COMMAND_REGISTER(CmdUnitTest, CommandId::unit_test_cmd);

#endif	// UNIT_TESTS