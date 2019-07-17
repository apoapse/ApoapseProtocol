#pragma once
#include "ApoapseData.h"
#include "Operation.h"

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
	bool propagateToOtherClients = false;
	bool clientUIPropagate = false;
	std::string clientUISignalName;
	bool receiveOnClient = false;
	bool receiveOnServer = false;
	bool saveOnReceive = false;

	bool operationRegister = false;
	OperationOwnership operationOwnership = OperationOwnership::all;

	void SetData(const DataStructure& data);
	DataStructure& GetData();
	bool IsValid(bool isAuthenticated) const;

	void Send(INetworkSender& destination, TCPConnection* excludedConnection = nullptr);

private:
};