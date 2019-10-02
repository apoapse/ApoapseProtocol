#pragma once
#include <vector>
#include <array>
#include <string>
#include <functional>
#include <map>
#include <optional>
#include "ByteUtils.hpp"
#include "TypeDefs.hpp"
#include "Maths.hpp"
#include "Range.hpp"
#include "StringExtensions.h"
#include "Diagnostic.hpp"

#ifdef _MSC_VER
#pragma warning (disable : 4310 )
#endif // _MSC_VER

using namespace std::literals::string_literals;	// Force the use of the string literal suffix

enum class FormatFirstByte
{
	nil = 0xc0,
	bfalse = 0xc2,
	btrue = 0xc3,

	// Byte array
	bin_8 = 0xc4,
	bin_16 = 0xc5,
	bin_32 = 0xc6,

	// tuple
	ext_8 = 0xc7,
	ext_16 = 0xc8,
	ext_32 = 0xc9,

	// map
	fix_map_low_bound = 0x80,
	fix_map_high_bound = 0x8f,
	map_16 = 0xde,
	map_32 = 0xdf,

	// Numbers
	positive_fixint_low_bound = 0x00,
	positive_fixint_high_bound = 0x7f,
	negative_fixint_high_bound = 0xe0,
	negative_fixint_low_bound = 0xff,
	float_32 = 0xca,
	float_64 = 0xcb,
	uint_8 = 0xcc,
	uint_16 = 0xcd,
	uint_32 = 0xce,
	uint_64 = 0xcf,
	int_8 = 0xd0,
	int_16 = 0xd1,
	int_32 = 0xd2,
	int_64 = 0xd3,

	// Fixed size tuple
	//FIXEXT_1					= 0xd4,
	//FIXECT_2					= 0xd5,
	//FIXECT_4					= 0xd6,
	//FIXECT_8					= 0xd7,
	//FIXECT_16					= 0xd8,

	// Byte array (intended for UTF-8 encoded text)
	fix_str_low_bound = 0xa0,
	fix_str_high_bound = 0xbf,
	str_8 = 0xd9,
	str_16 = 0xda,
	str_32 = 0xdb,

	// Json type array
	fixarray_low_bound = 0x90,
	fixarray_high_bound = 0x9f,
	array_16 = 0xdc,
	array_32 = 0xdd,
	//MAP_16					= 0xde,
	//MAP_32					= 0xdf,

};

class MessgePackInvalidFormat : public std::exception
{
	const std::string m_message;

public:
	MessgePackInvalidFormat(const char* message) : m_message(std::string(message))
	{
	}

	const char* what() const noexcept override
	{
		return m_message.c_str();
	}
};

#define MSGPK_ORDERED_APPEND(_serializer, _name, _data)					[&] { _serializer.OrderedAppend(_name, _data); }
#define MSGPK_ORDERED_GROUP(_serializer, _name)							[&] { _serializer.Group(_name,
#define MSGPK_ORDERED_END_GROUP											); },
#define MSGPK_ORDERED_APPEND_ARRAY(_serializer, _type, _name, _vector)	[&] { _serializer.OrderedAppendArray<_type>(_name, _vector); }

class MessagePackSerializer
{
	std::vector<byte> m_data;
	bool m_fieldsCompiled{ true };	// Used to automatically call UnorderedFieldsCompile() i fand when needed

	std::optional<
		std::map<std::string, std::vector<ByteContainer>>
	> m_unorderedFields;

public:
	MessagePackSerializer() = default;

#pragma region Ordered
	void Group(const std::string& name, std::initializer_list<std::function<void()>> functions)
	{
		StartGroupOrdered(functions.size(), name);

		for (const auto& function : functions)
			function();
	}

	void OrderedAppend(const std::string& name, MessagePackSerializer& outsideSerializer)
	{
		OrderedAppend(name, outsideSerializer.GetMessagePackBytes());
	}

	void OrderedAppend(const std::string& name, const std::vector<byte>& data)
	{
		AppendItem(name);
		AppendItem(data);
	}

	template <size_t SIZE>
	void OrderedAppend(const std::string& name, const std::array<byte, SIZE>& data)
	{
		OrderedAppend(name, std::vector<byte>(data.begin(), data.end()));
	}

	void OrderedAppend(const std::string& name, const std::string& str)
	{
		AppendItem(name);
		AppendItem(str);
	}

	template <typename T>
	void OrderedAppendArray(const std::string& name, const std::vector<T>& data)
	{
		AppendItem(name);
		AppendArray(data);
	}

	template <typename T>
	void OrderedAppend(const std::string& name, T data)
	{
		AppendItem(name);
		AppendItem(data);
	}
#pragma endregion Ordered

#pragma region Unordered
	template <typename T>
	void UnorderedAppend(const std::string& name, T data)
	{
		const auto names = ReadFieldGroupAndName(name);
		const auto fieldName = names.second;

		OrderedAppend(fieldName, data);
		ProcessNewUnorderedField(names);

		m_fieldsCompiled = false;
	}

	template <typename T>
	void UnorderedAppendArray(const std::string& name, const std::vector<T>& arr)
	{
		const auto names = ReadFieldGroupAndName(name);
		const auto fieldName = names.second;

		OrderedAppendArray(fieldName, arr);
		ProcessNewUnorderedField(names);

		m_fieldsCompiled = false;
	}

	void UnorderedAppendArray(const std::string& name, std::vector<MessagePackSerializer> outsideSerializers)
	{
		std::vector<ByteContainer> msgPackCompiled(outsideSerializers.size());
		for (size_t i = 0; i < msgPackCompiled.size(); i++)
		{
			msgPackCompiled[i] = outsideSerializers[i].GetMessagePackBytes();
		}

		UnorderedAppendArray<ByteContainer>(name, msgPackCompiled);
	}

private:
	void UnorderedFieldsCompile()
	{
		ASSERT_MSG(m_unorderedFields.has_value(), "No unordered data to compile");

		StartGroupOrdered((m_unorderedFields->size() - 1) + CountItemsInRootNode(), "");	// Root node, we remove 1 to the total count because CountItemsInRootNode() will include what's inside

		for (const auto& group : m_unorderedFields.value())
		{
			const auto groupName = group.first;
			if (!groupName.empty())
				StartGroupOrdered(group.second.size(), groupName);

			for (const auto& field : group.second)
			{
				m_data.insert(m_data.end(), field.begin(), field.end());
			}
		}

		m_unorderedFields.reset();
		m_fieldsCompiled = true;
	}
#pragma endregion Unordered

public:
	std::vector<byte>& GetMessagePackBytes()
	{
		if (!m_fieldsCompiled)
			UnorderedFieldsCompile();

		return m_data;
	}

private:
	void ProcessNewUnorderedField(const std::pair<std::string, std::string>& names)
	{
		if (!m_unorderedFields)
			m_unorderedFields = std::map<std::string, std::vector<ByteContainer>>();

		const auto[groupName, fieldName] = names;

		ASSERT_MSG(!fieldName.empty(), "The field name cannot be empty");

		const std::vector<byte> dataCopy = std::move(m_data);
		InsertUnorderedField(groupName, dataCopy);
	}

	inline void InsertUnorderedField(const std::string& groupName, const ByteContainer& data)
	{
		m_unorderedFields.value()[groupName].push_back(data);
	}

	inline size_t CountItemsInRootNode() const
	{
		if (m_unorderedFields->count("") == 0)
			return m_unorderedFields->size();
		else
			return m_unorderedFields->at("").size();
	}

	static std::pair<std::string, std::string> ReadFieldGroupAndName(const std::string& name)
	{
		const bool containsGroup = StringExtensions::contains(name, '.');

		std::string groupName = (containsGroup) ? name : std::string();
		std::string fieldName = name;

		if (containsGroup)
		{
			StringExtensions::erase_right(groupName, '.', true);
			StringExtensions::erase_left(fieldName, '.', true);
		}

		return std::make_pair(groupName, fieldName);
	}

	void StartGroupOrdered(size_t nbItems, const std::string& name)
	{
		if (!CanFit<UInt32>(nbItems))
			throw std::out_of_range("The number of items is too high to fit into a UInt32");

		if (name.length() > 0)
			AppendItem(name);

		if (IsInBound(nbItems + (size_t)FormatFirstByte::fix_map_low_bound, (size_t)FormatFirstByte::fix_map_low_bound, (size_t)FormatFirstByte::fix_map_high_bound))
		{
			m_data.push_back(static_cast<byte>((size_t)FormatFirstByte::fix_map_low_bound + nbItems));
		}
		else
		{
			if (CanFit<UInt16>(nbItems))
				InsertBytes<UInt16, FormatFirstByte::map_16>(ToBytes<UInt16>((UInt16)nbItems, Endianness::AP_BIG_ENDIAN));

			else if (CanFit<UInt32>(nbItems))
				InsertBytes<UInt32, FormatFirstByte::map_32>(ToBytes<UInt32>((UInt32)nbItems, Endianness::AP_BIG_ENDIAN));
		}
	}

	inline void AppendItem(bool value)
	{
		(value) ? m_data.push_back((byte)FormatFirstByte::btrue) : m_data.push_back((byte)FormatFirstByte::bfalse);
	}

	void AppendItem(const std::string& str)
	{
		if (!CanFit<UInt32>(str.length()))
			throw std::out_of_range("The string size is too high to fit into a UInt32");

		if (IsInBound(str.length() + (size_t)FormatFirstByte::fix_str_low_bound, (size_t)FormatFirstByte::fix_str_low_bound, (size_t)FormatFirstByte::fix_str_high_bound))
		{
			m_data.push_back(static_cast<byte>((size_t)FormatFirstByte::fix_str_low_bound + str.length()));
		}
		else
		{
			if (CanFit<UInt8>(str.length()))
				InsertBytes<UInt8, FormatFirstByte::str_8>(ToBytes<UInt8>((UInt8)str.length(), Endianness::AP_BIG_ENDIAN));

			else if (CanFit<UInt16>(str.length()))
				InsertBytes<UInt16, FormatFirstByte::str_16>(ToBytes<UInt16>((UInt16)str.length(), Endianness::AP_BIG_ENDIAN));

			else
				InsertBytes<UInt32, FormatFirstByte::str_32>(ToBytes<UInt32>((UInt32)str.length(), Endianness::AP_BIG_ENDIAN));
		}

		m_data.insert(m_data.end(), str.begin(), str.end());
	}

	void AppendItem(const std::vector<byte>& data)
	{
		if (!CanFit<UInt32>(data.size()))
			throw std::out_of_range("The byte array size is too high to fit into a UInt32");

		if (CanFit<UInt8>(data.size()))
			InsertBytes<UInt8, FormatFirstByte::bin_8>(ToBytes<UInt8>((UInt8)data.size(), Endianness::AP_BIG_ENDIAN));

		else if (CanFit<UInt16>(data.size()))
			InsertBytes<UInt16, FormatFirstByte::bin_16>(ToBytes<UInt16>((UInt16)data.size(), Endianness::AP_BIG_ENDIAN));

		else
			InsertBytes<UInt32, FormatFirstByte::bin_32>(ToBytes<UInt32>((UInt32)data.size(), Endianness::AP_BIG_ENDIAN));

		m_data.insert(m_data.end(), data.begin(), data.end());
	}

	template <typename T>
	void AppendArray(const std::vector<T>& data)
	{
		assert(CanFit<UInt32>(data.size()));

		if (CanFit<UInt8>(data.size()) && IsInBound((size_t)FormatFirstByte::fixarray_low_bound + data.size(), (size_t)FormatFirstByte::fixarray_low_bound, (size_t)FormatFirstByte::fixarray_high_bound))
			m_data.push_back(static_cast<byte>((size_t)FormatFirstByte::fixarray_low_bound + data.size()));

		else if (CanFit<UInt16>(data.size()))
			InsertBytes<UInt16, FormatFirstByte::array_16>(ToBytes<UInt16>((UInt16)data.size(), Endianness::AP_BIG_ENDIAN));

		else
			InsertBytes<UInt32, FormatFirstByte::array_32>(ToBytes<UInt32>((UInt32)data.size(), Endianness::AP_BIG_ENDIAN));

		for (const auto& item : data)
			AppendItem(item);
	}

	template <typename T>
	void AppendItem(T data)	// Numbers
	{
		if (data >= 0 && IsInBound(data, (T)FormatFirstByte::positive_fixint_low_bound, (T)FormatFirstByte::positive_fixint_high_bound))
		{
			m_data.push_back(static_cast<byte>((UInt8)data));	// The number can be represented as a single byte
		}
		else if (data < 0 && CanFit<Int8>(data) && IsInBound((Int8)data, (Int8)FormatFirstByte::negative_fixint_high_bound, (Int8)FormatFirstByte::negative_fixint_low_bound))	// IsInBound args inverted because we expect a negative number
		{
			m_data.push_back(static_cast<byte>((Int8)data));
		}
		else
		{
			if (CanFit<UInt8>(data)) // #CPP17 replace all instances of CanFit() if statement by if constexpr
			{
				m_data.push_back((byte)FormatFirstByte::uint_8);
				m_data.push_back(static_cast<byte>(data));
			}

			else if (CanFit<UInt16>(data))
			{
				const auto byteArray = ToBytes<UInt16>(static_cast<UInt16>(data), Endianness::AP_BIG_ENDIAN);

				InsertBytes<UInt16, FormatFirstByte::uint_16>(byteArray);
			}

			else if (CanFit<UInt32>(data))
			{
				const auto byteArray = ToBytes<UInt32>(static_cast<UInt32>(data), Endianness::AP_BIG_ENDIAN);

				InsertBytes<UInt32, FormatFirstByte::uint_32>(byteArray);
			}

			else if (CanFit<Int8>(data))
			{
				m_data.push_back((byte)FormatFirstByte::int_8);
				m_data.push_back(static_cast<byte>(data));
			}

			else if (CanFit<Int16>(data))
			{
				const auto byteArray = ToBytes<Int16>(static_cast<Int16>(data), Endianness::AP_BIG_ENDIAN);

				InsertBytes<Int16, FormatFirstByte::int_16>(byteArray);
			}

			else if (CanFit<int>(data))
			{
				const auto byteArray = ToBytes<int>(static_cast<int>(data), Endianness::AP_BIG_ENDIAN);

				InsertBytes<int, FormatFirstByte::int_32>(byteArray);
			}

			else if (CanFit<UInt64>(data))
			{
				const auto byteArray = ToBytes<UInt64>(static_cast<UInt64>(data), Endianness::AP_BIG_ENDIAN);

				InsertBytes<UInt64, FormatFirstByte::uint_64>(byteArray);
			}

			else
			{
				const auto byteArray = ToBytes<Int64>(static_cast<Int64>(data), Endianness::AP_BIG_ENDIAN);

				InsertBytes<Int64, FormatFirstByte::int_64>(byteArray);
			}
		}

	}

	template <typename ORIGINAL_TYPE, FormatFirstByte FIRST_BYTE>
	inline void InsertBytes(const std::array<byte, sizeof(ORIGINAL_TYPE)>& byteArray)
	{
		m_data.push_back((byte)FIRST_BYTE);
		m_data.insert(m_data.end(), byteArray.begin(), byteArray.end());
	}
};

class MessagePackDeserializer
{
	enum class ItemType
	{
		undefined,
		boolean,
		positive_fix_uint,
		negative_fix_int,
		uint8,
		uint16,
		uint32,
		uint64,
		int8,
		int16,
		int32,
		int64,
		text,
		bytes_blob,
		array_size_helper
	};

	struct ItemLocation
	{
		size_t pos = 0;
		size_t length = 0;
		ItemType type = ItemType::undefined;
	};

	const std::vector<byte>& m_rawData;
	const std::optional<std::vector<byte>> m_dataOwner;
	std::map<std::string, ItemLocation> m_parsedDataLocations;

	const std::vector<byte>& GetRawData() const
	{
		return (m_dataOwner.has_value()) ? m_dataOwner.value() : m_rawData;
	}

public:
	MessagePackDeserializer(const std::vector<byte>& data) : m_rawData(data)
	{
		Parse();
	}

	MessagePackDeserializer(const std::vector<byte>&& data)	// This constructor is used to make the deserializer owner of the raw data via a std::move, compaired to the normal constructor which take a reference where the lifetime of the raw data as to be managed by the user.
		: m_dataOwner(data)
		, m_rawData(data)
	{
		Parse();
	}

	MessagePackSerializer ToSerializer() const
	{
		MessagePackSerializer ser;

		for (const auto& item : m_parsedDataLocations)
		{
			if (item.second.type == ItemType::boolean)
			{
				ser.UnorderedAppend(item.first, GetValue<bool>(item.first));
			}
			else if (item.second.type == ItemType::text)
			{
				ser.UnorderedAppend(item.first, GetValue<std::string>(item.first));
			}
			else if (item.second.type == ItemType::bytes_blob)
			{
				ser.UnorderedAppend(item.first, GetValue<std::vector<byte>>(item.first));
			}
			else
			{
				ser.UnorderedAppend(item.first, GetValue<Int64>(item.first));
			}
		}
		
		return ser;
	}

	template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
	T GetValue(const std::string& key) const
	{
		const auto itemDef = m_parsedDataLocations.at(key);
		Range<std::vector<byte>> bytesRange(GetRawData());
		bytesRange.Consume(itemDef.pos);

		switch (itemDef.type)
		{
		case ItemType::positive_fix_uint:
			return (T)bytesRange[0];

		case ItemType::negative_fix_int:
			return (T)(Int8)bytesRange[0];

		case ItemType::uint8:
			return ReadAndCheckInteger<UInt8, T>(bytesRange);

		case ItemType::uint16:
			return ReadAndCheckInteger<UInt16, T>(bytesRange);

		case ItemType::uint32:
			return ReadAndCheckInteger<UInt32, T>(bytesRange);

		case ItemType::uint64:
			return ReadAndCheckInteger<UInt64, T>(bytesRange);

		case ItemType::int8:
			return ReadAndCheckInteger<Int8, T>(bytesRange);

		case ItemType::int16:
			return ReadAndCheckInteger<Int16, T>(bytesRange);

		case ItemType::int32:
			return ReadAndCheckInteger<int, T>(bytesRange);

		case ItemType::int64:
			return ReadAndCheckInteger<Int64, T>(bytesRange);

		default:
			throw std::runtime_error("Cannot convert the requested value");
		}
	}

	template<>
	bool GetValue(const std::string& key) const
	{
		const auto itemDef = m_parsedDataLocations.at(key);
		if (itemDef.type != ItemType::boolean)
			throw MessgePackInvalidFormat("The requested value is not of type BOOL");

		return (GetRawData().at(itemDef.pos) == (byte)FormatFirstByte::btrue);
	}

	template<>
	std::string GetValue(const std::string& key) const
	{
		const auto itemDef = m_parsedDataLocations.at(key);
		if (itemDef.type != ItemType::text)
			throw MessgePackInvalidFormat("The requested value is not of type TEXT");

		return std::string(GetRawData().begin() + itemDef.pos, GetRawData().begin() + itemDef.pos + itemDef.length);
	}

	template<>
	std::vector<byte> GetValue(const std::string& key) const
	{
		const auto itemDef = m_parsedDataLocations.at(key);
		if (itemDef.type != ItemType::bytes_blob)
			throw MessgePackInvalidFormat("The requested value is not of type BYTES_BLOB");

		return std::vector<byte>(GetRawData().begin() + itemDef.pos, GetRawData().begin() + itemDef.pos + itemDef.length);
	}


	template<>
	MessagePackDeserializer GetValue(const std::string& key) const
	{
		const auto itemDef = m_parsedDataLocations.at(key);
		if (itemDef.type != ItemType::bytes_blob)
			throw MessgePackInvalidFormat("The requested value is not of type BYTES_BLOB");

		auto bytes = std::vector<byte>(GetRawData().begin() + itemDef.pos, GetRawData().begin() + itemDef.pos + itemDef.length);

		return MessagePackDeserializer(std::move(bytes));
	}

	Range<std::vector<byte>> GetByteBlobRange(const std::string& key) const
	{
		const auto itemDef = m_parsedDataLocations.at(key);
		if (itemDef.type != ItemType::bytes_blob)
			throw MessgePackInvalidFormat("The requested value is not of type BYTES_BLOB");

		Range<std::vector<byte>> range(GetRawData(), itemDef.pos + itemDef.length);
		range.Consume(itemDef.pos);

		return range;
	}

	template <typename T>
	std::optional<T> GetValueOptional(const std::string& key) const
	{
		return (m_parsedDataLocations.count(key) > 0) ? GetValue<T>(key) : std::optional<T>();
	}

	template <typename T>
	std::vector<T> GetArray(const std::string& key) const
	{
		std::vector<T> output;
		const auto arrayDef = m_parsedDataLocations.at(key);

		if (arrayDef.type != ItemType::array_size_helper)
			throw MessgePackInvalidFormat("The requested item is not an array");

		for (size_t i = 0; i < arrayDef.length; i++)
			output.push_back(GetValue<T>(key + "." + std::to_string(i)));

		return output;
	}
	template <typename T>
	std::optional<std::vector<T>> GetArrayOptional(const std::string& key) const
	{
		std::optional<std::vector<T>> output;

		if (m_parsedDataLocations.count(key) > 0)
			output = GetArray<T>(key);

		return output;
	}

	bool Exist(const std::string& key) const
	{
		return (m_parsedDataLocations.count(key) > 0);
	}

private:
	void Parse()
	{
		Range<std::vector<byte>> range(GetRawData());

		ParseMap(range, "");
	}

	void ParseMap(Range<ByteContainer>& workingRange, const std::string& currentMapName)
	{
		const byte currentFirstByte = workingRange[0];
		workingRange.Consume(1);

		UInt32 nbItems = 0;

		if (currentFirstByte == (byte)FormatFirstByte::map_16)
		{
			if (workingRange.size() < sizeof(UInt16))
				throw MessgePackInvalidFormat("Byte sequence too short for UInt16");

			nbItems = (UInt32)ReadInteger<UInt16>(workingRange);
			workingRange.Consume(sizeof(UInt16));
		}

		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::fix_map_low_bound, (byte)FormatFirstByte::fix_map_high_bound))
		{
			nbItems = static_cast<UInt32>((UInt8)currentFirstByte - (UInt8)FormatFirstByte::fix_map_low_bound);
		}

		else if (currentFirstByte == (byte)FormatFirstByte::map_32)
		{
			if (workingRange.size() < sizeof(UInt32))
				throw MessgePackInvalidFormat("Byte sequence too short for UInt32");

			nbItems = ReadInteger<UInt32>(workingRange);
			workingRange.Consume(sizeof(UInt32));
		}
		else
			return;

		if (nbItems == 0)
			return;

		// Parse each key/values pair of the map
		for (size_t i = 0; i < nbItems; i++)
		{
			const std::string key = ParseText(workingRange, currentMapName, true);

			ParseNextItem(workingRange, key);
		}
	}

	void ParseNextItem(Range<ByteContainer>& workingRange, const std::string& currentMapName, bool allowMap = true)
	{
		if (workingRange.size() == 0)
			throw MessgePackInvalidFormat("Byte sequence too short");

		const byte currentFirstByte = workingRange[0];

		// Map
		if (allowMap && IsFirstByteMapIndex(currentFirstByte))
			ParseMap(workingRange, currentMapName);

		// Text
		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::fix_str_low_bound, (byte)FormatFirstByte::fix_str_high_bound))
			ParseText(workingRange, currentMapName, false);

		else if (currentFirstByte == (byte)FormatFirstByte::str_8)
			ParseText(workingRange, currentMapName, false);

		else if (currentFirstByte == (byte)FormatFirstByte::str_16)
			ParseText(workingRange, currentMapName, false);

		else if (currentFirstByte == (byte)FormatFirstByte::str_32)
			ParseText(workingRange, currentMapName, false);

		// bool
		else if (currentFirstByte == (byte)FormatFirstByte::bfalse || currentFirstByte == (byte)FormatFirstByte::btrue)
		{
			ItemLocation item;
			item.pos = workingRange.GetCursorPosition();
			item.type = ItemType::boolean;

			m_parsedDataLocations[currentMapName] = item;

			workingRange.Consume(1);
		}

		// Integer
		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::positive_fixint_low_bound, (byte)FormatFirstByte::positive_fixint_high_bound))
		{
			ItemLocation item;
			item.pos = workingRange.GetCursorPosition();
			item.type = ItemType::positive_fix_uint;

			m_parsedDataLocations[currentMapName] = item;

			workingRange.Consume(1);
		}

		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::negative_fixint_high_bound, (byte)FormatFirstByte::negative_fixint_low_bound))
		{
			ItemLocation item;
			item.pos = workingRange.GetCursorPosition();
			item.type = ItemType::negative_fix_int;

			m_parsedDataLocations[currentMapName] = item;

			workingRange.Consume(1);
		}

		else if (currentFirstByte == (byte)FormatFirstByte::uint_8)
			ParseInteger<UInt8>(workingRange, ItemType::uint8, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::uint_16)
			ParseInteger<UInt16>(workingRange, ItemType::uint16, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::uint_32)
			ParseInteger<UInt32>(workingRange, ItemType::uint32, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::uint_64)
			ParseInteger<UInt64>(workingRange, ItemType::uint64, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::int_8)
			ParseInteger<Int8>(workingRange, ItemType::int8, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::int_16)
			ParseInteger<Int16>(workingRange, ItemType::int16, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::int_32)
			ParseInteger<int>(workingRange, ItemType::int32, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::int_64)
			ParseInteger<Int64>(workingRange, ItemType::int64, currentMapName);

		// Byte array
		else if (currentFirstByte == (byte)FormatFirstByte::bin_8 || currentFirstByte == (byte)FormatFirstByte::bin_16 || currentFirstByte == (byte)FormatFirstByte::bin_32)
			ParseByteBlob(workingRange, currentMapName);

		// Array
		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::fixarray_low_bound, (byte)FormatFirstByte::fixarray_high_bound) || currentFirstByte == (byte)FormatFirstByte::array_16 || currentFirstByte == (byte)FormatFirstByte::array_32)
		{
			ParseArray(workingRange, currentMapName);
		}
	}

	std::string ParseText(Range<ByteContainer>& workingRange, const std::string& keyName, bool isKey)
	{
		const byte currentFirstByte = workingRange[0];
		UInt32 strLength = 0;
		workingRange.Consume(1);

		// Parse
		if (IsInBound(currentFirstByte, (byte)FormatFirstByte::fix_str_low_bound, (byte)FormatFirstByte::fix_str_high_bound))
			strLength = static_cast<UInt8>(currentFirstByte - (byte)FormatFirstByte::fix_str_low_bound);

		else if (currentFirstByte == (byte)FormatFirstByte::str_8)
			strLength = (UInt32)ReadAndConsumeInteger<UInt8>(workingRange);

		else if (currentFirstByte == (byte)FormatFirstByte::str_16)
			strLength = (UInt32)ReadAndConsumeInteger<UInt16>(workingRange);

		else if (currentFirstByte == (byte)FormatFirstByte::str_32)
			strLength = (UInt32)ReadAndConsumeInteger<UInt16>(workingRange);

		// Process
		if (strLength == 0)
			return "";

		if (isKey)
		{
			if (strLength > 255)
				throw MessgePackInvalidFormat("This text is too large to be a key name");

			std::string nextMapName = (keyName == "") ? ReadText(workingRange, strLength) : keyName + "." + ReadText(workingRange, strLength);
			workingRange.Consume(strLength);

			return nextMapName;
		}
		else
		{
			ItemLocation item;
			item.length = strLength;
			item.pos = workingRange.GetCursorPosition();
			item.type = ItemType::text;

			m_parsedDataLocations[keyName] = item;
			workingRange.Consume(strLength);
			return "";
		}
	}

	void ParseArray(Range<ByteContainer>& workingRange, const std::string& keyName)
	{
		const byte currentFirstByte = workingRange[0];
		workingRange.Consume(1);
		UInt32 nbItems = 0;

		// Parse
		if (IsInBound(currentFirstByte, (byte)FormatFirstByte::fixarray_low_bound, (byte)FormatFirstByte::fixarray_high_bound))
			nbItems = static_cast<UInt8>(currentFirstByte - (byte)FormatFirstByte::fixarray_low_bound);

		else if (currentFirstByte == (byte)FormatFirstByte::array_16)
			nbItems = (UInt32)ReadAndConsumeInteger<UInt16>(workingRange);

		else if (currentFirstByte == (byte)FormatFirstByte::array_32)
			nbItems = (UInt32)ReadAndConsumeInteger<UInt32>(workingRange);

		// Add helper
		ItemLocation item;
		item.type = ItemType::array_size_helper;
		item.pos = 0;
		item.length = nbItems;
		m_parsedDataLocations[keyName] = item;

		// Parse the contained items
		for (size_t i = 0; i < nbItems; i++)
			ParseNextItem(workingRange, keyName + "." + std::to_string(i), false);
	}

	void ParseByteBlob(Range<ByteContainer>& workingRange, const std::string& keyName)
	{
		const byte currentFirstByte = workingRange[0];
		workingRange.Consume(1);

		UInt32 blobLength = 0;

		if (currentFirstByte == (byte)FormatFirstByte::bin_8)
			blobLength = (UInt32)ReadAndConsumeInteger<UInt8>(workingRange);

		if (currentFirstByte == (byte)FormatFirstByte::bin_16)
			blobLength = (UInt32)ReadAndConsumeInteger<UInt16>(workingRange);

		if (currentFirstByte == (byte)FormatFirstByte::bin_32)
			blobLength = ReadAndConsumeInteger<UInt32>(workingRange);

		ItemLocation item;
		item.length = blobLength;
		item.pos = workingRange.GetCursorPosition();
		item.type = ItemType::bytes_blob;
		workingRange.Consume(blobLength);

		m_parsedDataLocations[keyName] = item;
	}

	template <typename T>
	void ParseInteger(Range<ByteContainer>& workingRange, ItemType itemType, const std::string& keyName)
	{
		workingRange.Consume(1); // Consume the type specifier

		ItemLocation item;
		item.pos = workingRange.GetCursorPosition();
		item.type = itemType;

		m_parsedDataLocations[keyName] = item;
		workingRange.Consume(sizeof(T));
	}

	template <typename SOURCE_TYPE, typename TARGET_TYPE>
	TARGET_TYPE ReadAndCheckInteger(Range<ByteContainer>& bytesRange) const
	{
		auto value = ReadInteger<SOURCE_TYPE>(bytesRange);

		if (CanFit<TARGET_TYPE>(value))
			return static_cast<TARGET_TYPE>(value);
		else
			throw std::out_of_range("The value do not fit into the specified type");
	}

	template <typename T>
	T ReadAndConsumeInteger(Range<ByteContainer>& bytesRange)
	{
		T value = ReadInteger<T>(bytesRange);
		bytesRange.Consume(sizeof(T));

		return value;
	}

	bool IsFirstByteMapIndex(byte nextByte) const
	{
		return (nextByte == (byte)FormatFirstByte::map_16 || IsInBound(nextByte, (byte)FormatFirstByte::fix_map_low_bound, (byte)FormatFirstByte::fix_map_high_bound) || nextByte == (byte)FormatFirstByte::map_32);
	}

	template <typename T>
	static T ReadInteger(const Range<ByteContainer>& bytesRange)
	{
		std::array<byte, sizeof(T)> bytes;
		std::copy(bytesRange.begin(), bytesRange.begin() + sizeof(T), bytes.begin());

		return FromBytes<T>(bytes, Endianness::AP_BIG_ENDIAN);
	}

	static std::string ReadText(const Range<ByteContainer>& workingRange, UInt32 length)
	{
		if (length > workingRange.size())
			throw MessgePackInvalidFormat("Byte sequence too short");

		return std::string(workingRange.begin(), workingRange.begin() + length);
	}
};

#ifdef _MSC_VER
#pragma warning (default : 4310 )
#endif // _MSC_VER