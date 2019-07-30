#pragma once
#include "ApoapseData.h"
#include "INetworkSender.h"

enum class OperationOwnership
{
	undefined,
	all,
	self,
	usergroup,
};

class CommandV2
{
	DataStructure m_data;

public:
	CommandV2() = default;
	CommandV2(DataStructure& data);

	std::string name;
	std::string nameShort;
	std::string relatedDataStructure;

	bool requireAuthentication = true;
	bool onlyNonAuthenticated = false;

	bool clientUIPropagate = false;
	std::string clientUISignalName;

	bool receiveOnClient = false;
	bool receiveOnServer = false;
	bool saveOnReceive = false;

	bool propagate = false;
	bool excludeSelfPropagation = false;

	bool operationRegister = false;
	OperationOwnership operationOwnership = OperationOwnership::all;
	static OperationOwnership ConvertFieldToOwnership(const std::string& value);

	void SetData(const DataStructure& data);
	DataStructure& GetData();
	bool IsValid(bool isAuthenticated) const;

	void Send(INetworkSender& destination, TCPConnection* excludedConnection = nullptr);

private:
};