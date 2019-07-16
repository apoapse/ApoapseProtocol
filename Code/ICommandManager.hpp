#pragma once
class CommandV2;
class NetworkPayload;
class GenericConnection;

using CommandV2Def = CommandV2;

struct ICommandManager
{
	virtual ~ICommandManager() = default;

	virtual CommandV2 CreateCommand(std::shared_ptr<NetworkPayload> netPayload) = 0;
	virtual CommandV2 CreateCommand(const std::string& cmdName, const DataStructure& data) = 0;
	virtual bool CommandExist(const std::string& cmdShortName) const = 0;
	virtual void OnReceivedCmdInternal(CommandV2& cmd, GenericConnection& connection, void* relatedUser = nullptr) = 0;
	virtual const CommandV2Def& GetCmdDefByFullName(const std::string& name) const = 0;
};