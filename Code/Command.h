#pragma once
#include <functional>
#include <vector>
#include <string>
#include <optional>
#include "MessagePack.hpp"
#include "ByteUtils.hpp"
#include "INetworkSender.h"
#include "Commands.hpp"

struct NetworkPayload;

struct IFieldValidator
{
	virtual bool HasValue(const std::string& fieldName, const MessagePackDeserializer& deserializer) const = 0;
	virtual bool HasValidatorFunction() const = 0;
	virtual bool ExecValidator(const std::string& fieldName, const MessagePackDeserializer& deserialize) const = 0;
};

template <typename T>
class FieldValueValidator : public IFieldValidator
{
	std::optional<std::function<bool(T)>> m_validatorFunction;
	
public:
	FieldValueValidator() = default;

	FieldValueValidator(std::function<bool(T)> validatorFunction) : m_validatorFunction(validatorFunction)
	{
	}

	bool HasValue(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		return deserializer.GetValueOptional<T>(fieldName).has_value();
	}

	bool HasValidatorFunction() const override
	{
		return m_validatorFunction.has_value();
	}

	bool ExecValidator(const std::string& fieldName, const MessagePackDeserializer& deserializer) const override
	{
		T value = deserializer.GetValue<T>(fieldName);

		return m_validatorFunction.value()(value);
	}
};

template <typename T>
class FieldArrayValidator : public IFieldValidator
{
	std::optional<std::function<bool(std::vector<T>)>> m_validatorFunction;

public:
	FieldArrayValidator() = default;

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
		std::vector<T> array = deserializer.GetArray<T>(fieldName);

		return m_validatorFunction.value()(array);
	}
};

#define FIELD_VALUE_VALIDATOR(_type, _func)			new FieldValueValidator<_type>(_func)
#define FIELD_VALUE(_type)							new FieldValueValidator<_type>([](_type){ return true; })

#define FIELD_ARRAY_VALIDATOR(_type, _func)			new FieldArrayValidator<_type>(_func)
#define FIELD_ARRAY(_type)							new FieldArrayValidator<_type>([](std::vector<_type>){ return true; })
//#define FIELD_VALUE_CHECK_TYPE(_type)				new FieldValueValidator<_type>([](_type){ return true; })	// WARNING: In the case of integers, FieldValueValidator::ConvertFromStr is only able to know if the value is a number or not without cheking his size or if it is signed or unsigned.
//#define PROCESS_METHOD(_inputType, _method)			[this](_inputType& input) { _method(input); };
//#define PROCESS_METHOD_FROM_USER(_method)			[this](LocalUser& user, ClientConnection& connection) { return _method(user, connection); };

enum class FieldRequirement
{
	ANY_OPTIONAL,
	ANY_MENDATORY
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
	Commands command;
	std::vector<CommandField> fields;
// 	std::function<void(ClientConnection&)> processFromClient = { NULL };
// 	std::function<bool(LocalUser&, ClientConnection&)> processFromUser = { NULL };
// 	std::function<void(RemoteServer&)> processFromRemoteServer = { NULL };
	bool propagateToUser = { false };

	~CommandInfo()
	{
		for (auto& field : fields)
			delete field.fieldValueValidator;
	}
};

class Command
{
	std::shared_ptr<NetworkPayload> m_rawData;
	bool m_isValid{ false };

protected:
	std::unique_ptr<MessagePackDeserializer> m_deserializedData;

public:
	void Parse(std::shared_ptr<NetworkPayload> data);
	bool IsValid() const;
	virtual CommandInfo& GetInfo() const = 0;

private:
	void AutoValidate();
	bool ValidateField(const CommandField& field, bool valueExist);
};