#pragma once
class CommandV2;
class NetworkPayload;

struct ICommandManager
{
	virtual ~ICommandManager() = default;

	virtual CommandV2 CreateCommand(std::shared_ptr<NetworkPayload> netPayload) = 0;
	virtual CommandV2 CreateCommand(const std::string& cmdName, const DataStructure& data) = 0;
	virtual bool CommandExist(const std::string& cmdShortName) const = 0;
};