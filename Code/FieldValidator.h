#pragma once
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
	{}

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
	{}

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

struct Field
{
	std::string name;
	FieldRequirement requirement;
	IFieldValidator* fieldValueValidator;

	template <typename T>
	static bool ContainerIsNotEmpty(const T& arr)
	{
		return (arr.size() > 0);
	}

	static bool DoesFieldHaveValue(const Field& field, const MessagePackDeserializer& source);
	static bool ValidateField(const Field& field, bool valueExist, const MessagePackDeserializer& source);
};

