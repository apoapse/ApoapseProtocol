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
	bool propagateToClientUI = false;

	bool operationRegister = false;
	OperationOwnership operationOwnership = OperationOwnership::all;

	void SetData(const DataStructure& data);
	bool IsValid() const;

	void Send(INetworkSender& destination, TCPConnection* excludedConnection = nullptr);

private:
};