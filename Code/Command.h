#pragma once
#include <functional>
#include <vector>
#include <string>
#include <optional>
#include "MessagePack.hpp"
#include "ByteUtils.hpp"
#include "INetworkSender.h"
#include "Commands.hpp"
#include "ApoapseMetadata.h"
#include "FieldValidator.h"

#pragma warning( disable : 4266)

class NetworkPayload;
class User;
class GenericConnection;
class ServerConnection;
class ClientConnection;


struct CommandInfo
{
	//std::string name;
	CommandId command;
	std::vector<Field> fields;
	bool serverOnly = { false };
	bool clientOnly = { false };
	bool requireAuthentication = { false };
	bool onlyNonAuthenticated = { false };
	bool allowForUsersRequiredToChangePassword = { false };

	int metadataTypes = 0;
	std::vector<Field> metadataSelfFields;
	std::vector<Field> metadataUsergroupFields;
	std::vector<Field> metadataAllFields;

	~CommandInfo()
	{
		for (auto& field : fields)
			delete field.fieldValueValidator;
	}
};

class Command
{
	bool m_isValid{ false };
	std::unique_ptr<MessagePackDeserializer> m_deserializedData;
	std::vector<byte> m_networkPayload;

public:
	virtual ~Command() = default;

	void Parse(std::shared_ptr<NetworkPayload> payload);
	bool IsValid() const;
	virtual CommandInfo& GetInfo() const = 0;

	virtual void Process(ServerConnection& sender);
	virtual void Process(User& sender, ServerConnection& senderConnection);
	virtual void Process(ClientConnection& sender);

	// Use to send from the outside a command where the content has already been set internally in m_serializedData
	void Send(INetworkSender& destination, TCPConnection* excludedConnection = nullptr);

	void Propagate(INetworkSender& destination, TCPConnection* excludedConnection = nullptr);	// Propagate using the command original data
	void Propagate(MessagePackSerializer ser, INetworkSender& destination, TCPConnection* excludedConnection = nullptr);	// Propagate using custom/new data

protected:
	std::optional<MessagePackSerializer> m_serializedData;
	void Send(MessagePackSerializer& data, INetworkSender& destination, TCPConnection* excludedConnection = nullptr);
	const MessagePackDeserializer& GetFieldsData() const;

	ApoapseMetadata GetMetadataField(MetadataAcess metadataType);

private:
	void AutoValidate();
	bool ValidateMetadataFields() const;
};