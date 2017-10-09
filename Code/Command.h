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
class FieldValueValidator : public IFieldValidator
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
class FieldArrayValidator : public IFieldValidator
{
	std::function<bool(std::vector<T>)> m_validatorFunction;

public:
	FieldArrayValidator(std::function<bool(std::vector<T>)> validatorFunction) : m_validatorFunction(validatorFunction)
	{
	}

	bool HasValue(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		return deserializer.GetArrayOptional<T>(fieldName).has_value();
	}

	bool HasValidatorFunction() const override
	{
		return m_validatorFunction.has_value();
	}

	bool ExecValidator(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		try
		{
			std::vector<T> array = deserializer.GetArray<T>(fieldName);

			return m_validatorFunction.value()(array);
		}
		catch (const std::exception&)
		{
			return false;
		}
	}
};

#define FIELD_VALUE_VALIDATOR(_type, _func)			new FieldValueValidator<_type>(_func)
#define FIELD_VALUE(_type)							new FieldValueValidator<_type>([](_type){ return false; })

#define FIELD_ARRAY_VALIDATOR(_type, _func)			new FieldArrayValidator<_type>(_func)
#define FIELD_ARRAY(_type)							new FieldArrayValidator<_type>([](std::vector<_type>&){ return true; })
//#define FIELD_VALUE_CHECK_TYPE(_type)				new FieldValueValidator<_type>([](_type){ return true; })	// WARNING: In the case of integers, FieldValueValidator::ConvertFromStr is only able to know if the value is a number or not without cheking his size or if it is signed or unsigned.
//#define PROCESS_METHOD(_inputType, _method)			[this](_inputType& input) { _method(input); };
//#define PROCESS_METHOD_FROM_USER(_method)			[this](LocalUser& user, ClientConnection& connection) { return _method(user, connection); };

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
	bool propagateToSenderConnections = { false }; // #TODO 

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

public:
	virtual ~Command() = default;

	void Parse(std::shared_ptr<NetworkPayload> payload);
	bool IsValid() const;
	virtual CommandInfo& GetInfo() const = 0;

	virtual void Process(const ServerConnection& sender);
	virtual void Process(const User& sender, const ServerConnection& senderConnection);
	virtual void Process(const ClientConnection& sender);

	// Use to send from the outside a command where the content has already been set internally in m_serializedData
	void Send(INetworkSender& destination, TCPConnection* excludedConnection = nullptr);

protected:
	std::optional<MessagePackSerializer> m_serializedData;
	void Send(MessagePackSerializer& data, INetworkSender& destination, TCPConnection* excludedConnection = nullptr);

private:
	void AutoValidate();
	bool DoesFieldHaveValue(const CommandField &field) const;
	bool ValidateField(const CommandField& field, bool valueExist);
};