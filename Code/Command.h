#pragma once
#include <functional>
#include <vector>
#include <string>
#include <optional>
#include "MessagePack.hpp"
#include "ByteUtils.hpp"
#include "INetworkSender.h"
#include "Commands.hpp"

class NetworkPayload;
class User;
class GenericConnection;
class ServerConnection;
class ClientConnection;

struct IFieldValidator
{
	virtual bool HasValue(const std::string& fieldName, const MessagePackDeserializer& deserializer) const = 0;
	virtual bool ExecValidator(const std::string& fieldName, const MessagePackDeserializer& deserialize) const = 0;
};

template <typename T>
class FieldValueValidator final : public IFieldValidator
{
	std::function<bool(T)> m_validatorFunction;
	
public:
	FieldValueValidator(std::function<bool(T)> validatorFunction) : m_validatorFunction(validatorFunction)
	{
	}

	bool HasValue(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		return deserializer.GetValueOptional<T>(fieldName).has_value();
	}

	bool ExecValidator(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		try
		{
			T value = deserializer.GetValue<T>(fieldName);

			return m_validatorFunction(value);
		}
		catch (const std::exception&)
		{
			return false;
		}
	}
};

template <typename T>
class FieldArrayValidator final : public IFieldValidator
{
	const std::function<bool(std::vector<T>&)> m_validatorFunction;

public:
	FieldArrayValidator(std::function<bool(std::vector<T>&)> validatorFunction) : m_validatorFunction(validatorFunction)
	{
	}

	bool HasValue(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		return deserializer.GetArrayOptional<T>(fieldName).has_value();
	}

	bool ExecValidator(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		try
		{
			std::vector<T> value = deserializer.GetArray<T>(fieldName);

			return m_validatorFunction(value);
		}
		catch (const std::exception&)
		{
			return false;
		}
	}
};

#define FIELD_VALUE_VALIDATOR(_type, _func)			new FieldValueValidator<_type>(_func)
#define FIELD_VALUE(_type)							new FieldValueValidator<_type>([](_type){ return true; })

#define FIELD_ARRAY_VALIDATOR(_type, _func)			new FieldArrayValidator<_type>(_func)
#define FIELD_ARRAY(_type)							new FieldArrayValidator<_type>([](std::vector<_type>&){ return true; })

enum class FieldRequirement
{
	any_optional,
	any_mendatory
};

struct CommandField
{
	std::string name;
	FieldRequirement requirement;
	IFieldValidator* fieldValueValidator;

	template <typename T>
	static bool ContainerIsNotEmpty(const T& arr)
	{
		return (arr.size() > 0);
	}
};

struct CommandInfo
{
	//std::string name;
	CommandId command;
	std::vector<CommandField> fields;
	bool serverOnly = { false };
	bool clientOnly = { false };
	bool requireAuthentication = { false };
	bool onlyNonAuthenticated = { false };

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

private:
	void AutoValidate();
	bool DoesFieldHaveValue(const CommandField &field) const;
	bool ValidateField(const CommandField& field, bool valueExist);
};