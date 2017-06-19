#pragma once
#include <vector>
#include <string>
#include <functional>
#include <map>
#include <optional>
#include "TypeDefs.hpp"
#include "Maths.hpp"
#include "Range.hpp"
#include "ByteUtils.hpp"

enum class FormatFirstByte
{
	NIL = 0xc0,
	FALSE = 0xc2,
	TRUE = 0xc3,

	// Byte array
	BIN_8 = 0xc4,
	BIN_16 = 0xc5,
	BIN_32 = 0xc6,

	// tuple
	EXT_8 = 0xc7,
	EXT_16 = 0xc8,
	EXT_32 = 0xc9,

	// map
	FIX_MAP_LOW_BOUND = 0x80,
	FIX_MAP_HIGH_BOUND = 0x8f,
	MAP_16 = 0xde,
	MAP_32 = 0xdf,

	// Numbers
	POSITIVE_FIXINT_LOW_BOUND = 0x00,
	POSITIVE_FIXINT_HIGH_BOUND = 0x7f,
	NEGATIVE_FIXINT_HIGH_BOUND = 0xe0,
	NEGATIVE_FIXINT_LOW_BOUND = 0xff,
	FLOAT_32 = 0xca,
	FLOAT_64 = 0xcb,
	UINT_8 = 0xcc,
	UINT_16 = 0xcd,
	UINT_32 = 0xce,
	UINT_64 = 0xcf,
	INT_8 = 0xd0,
	INT_16 = 0xd1,
	INT_32 = 0xd2,
	INT_64 = 0xd3,

	// Fixed size tuple
	//FIXEXT_1					= 0xd4,
	//FIXECT_2					= 0xd5,
	//FIXECT_4					= 0xd6,
	//FIXECT_8					= 0xd7,
	//FIXECT_16					= 0xd8,

	// Byte array (intended for UTF-8 encoded text)
	FIX_STR_LOW_BOUND = 0xa0,
	FIX_STR_HIGH_BOUND = 0xbf,
	STR_8 = 0xd9,
	STR_16 = 0xda,
	STR_32 = 0xdb,

	// Json type array
	FIXARRAY_LOW_BOUND = 0x90,
	FIXARRAY_HIGH_BOUND = 0x9f,
	ARRAY_16 = 0xdc,
	ARRAY_32 = 0xdd,
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

	const char* what() const override
	{
		return m_message.c_str();
	}
};

#define MSGPK_ORDERED_APPEND(_serializer, _name, _data)			[&] { _serializer.OrderedAppend(_name, _data); }
#define MSGPK_ORDERED_GROUP(_serializer, _name)					[&] { _serializer.Group(_name,
#define MSGPK_ORDERED_END_GROUP									); },

class MessagePackSerializer
{
	std::vector<byte> m_data;

public:
	MessagePackSerializer() = default;

	void Group(const std::string& name, std::initializer_list<std::function<void()>> functions)
	{
		StartGroupOrdered(functions.size(), name);

		for (const auto& function : functions)
			function();
	}

	void OrderedAppend(const std::string& name, const std::vector<byte>& data)
	{
		AppendItem(name);
		AppendItem(data);
	}

	void OrderedAppend(const std::string& name, const char* str)
	{
		AppendItem(name);
		AppendItem(std::string(str));
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

	const std::vector<byte>& GetMessagePackBytes() const
	{
		return m_data;
	}

private:
	void StartGroupOrdered(size_t nbItems, const std::string& name)
	{
		if (!CanFit<UInt32>(nbItems))
			throw std::out_of_range("The number of items is too high to fit into a UInt32");

		if (name.length() > 0)
			AppendItem(name);

		if (IsInBound(nbItems + (size_t)FormatFirstByte::FIX_MAP_LOW_BOUND, (size_t)FormatFirstByte::FIX_MAP_LOW_BOUND, (size_t)FormatFirstByte::FIX_MAP_HIGH_BOUND))
		{
			m_data.push_back(static_cast<byte>((size_t)FormatFirstByte::FIX_MAP_LOW_BOUND + nbItems));
		}
		else
		{
			if (CanFit<UInt16>(nbItems))
				InsertBytes<UInt16, FormatFirstByte::MAP_16>(ToBytes<UInt16, Endianness::BIG_ENDIAN>((UInt16)nbItems));

			else if (CanFit<UInt32>(nbItems))
				InsertBytes<UInt32, FormatFirstByte::MAP_32>(ToBytes<UInt32, Endianness::BIG_ENDIAN>((UInt32)nbItems));
		}
	}

	inline void AppendItem(bool value)
	{
		(value) ? m_data.push_back((byte)FormatFirstByte::TRUE) : m_data.push_back((byte)FormatFirstByte::FALSE);
	}

	void AppendItem(const std::string& str)
	{
		if (!CanFit<UInt32>(str.length()))
			throw std::out_of_range("The string size is too high to fit into a UInt32");

		if (IsInBound(str.length() + (size_t)FormatFirstByte::FIX_STR_LOW_BOUND, (size_t)FormatFirstByte::FIX_STR_LOW_BOUND, (size_t)FormatFirstByte::FIX_STR_HIGH_BOUND))
		{
			m_data.push_back(static_cast<byte>((size_t)FormatFirstByte::FIX_STR_LOW_BOUND + str.length()));
		}
		else
		{
			if (CanFit<UInt8>(str.length()))
				InsertBytes<UInt8, FormatFirstByte::STR_8>(ToBytes<UInt8, Endianness::BIG_ENDIAN>((UInt8)str.length()));

			else if (CanFit<UInt16>(str.length()))
				InsertBytes<UInt16, FormatFirstByte::STR_16>(ToBytes<UInt16, Endianness::BIG_ENDIAN>((UInt16)str.length()));

			else
				InsertBytes<UInt32, FormatFirstByte::STR_32>(ToBytes<UInt32, Endianness::BIG_ENDIAN>((UInt32)str.length()));
		}

		m_data.insert(m_data.end(), str.begin(), str.end());
	}

	void AppendItem(const std::vector<byte>& data)
	{
		if (!CanFit<UInt32>(data.size()))
			throw std::out_of_range("The byte array size is too high to fit into a UInt32");

		if (CanFit<UInt8>(data.size()))
			InsertBytes<UInt8, FormatFirstByte::BIN_8>(ToBytes<UInt8, Endianness::BIG_ENDIAN>((UInt8)data.size()));

		else if (CanFit<UInt16>(data.size()))
			InsertBytes<UInt16, FormatFirstByte::BIN_16>(ToBytes<UInt16, Endianness::BIG_ENDIAN>((UInt16)data.size()));

		else
			InsertBytes<UInt32, FormatFirstByte::BIN_32>(ToBytes<UInt32, Endianness::BIG_ENDIAN>((UInt32)data.size()));

		m_data.insert(m_data.end(), data.begin(), data.end());
	}

	template <typename T>
	void AppendArray(const std::vector<T>& data)
	{
		assert(CanFit<UInt32>(data.size()));

		if (CanFit<UInt8>(data.size()) && IsInBound((size_t)FormatFirstByte::FIXARRAY_LOW_BOUND + data.size(), (size_t)FormatFirstByte::FIXARRAY_LOW_BOUND, (size_t)FormatFirstByte::FIXARRAY_HIGH_BOUND))
			m_data.push_back(static_cast<byte>((size_t)FormatFirstByte::FIXARRAY_LOW_BOUND + data.size()));

		else if (CanFit<UInt16>(data.size()))
			InsertBytes<UInt16, FormatFirstByte::ARRAY_16>(ToBytes<UInt16, Endianness::BIG_ENDIAN>((UInt16)data.size()));

		else
			InsertBytes<UInt32, FormatFirstByte::ARRAY_32>(ToBytes<UInt32, Endianness::BIG_ENDIAN>((UInt32)data.size()));

		for (const auto& item : data)
			AppendItem(item);
	}

	template <typename T>
	void AppendItem(T data)	// Numbers
	{
		if (data >= 0 && IsInBound(data, (T)FormatFirstByte::POSITIVE_FIXINT_LOW_BOUND, (T)FormatFirstByte::POSITIVE_FIXINT_HIGH_BOUND))
		{
			m_data.push_back(static_cast<byte>((UInt8)data));	// The number can be represented as a single byte
		}
		else if (data < 0 && CanFit<Int8>(data) && IsInBound((Int8)data, (Int8)FormatFirstByte::NEGATIVE_FIXINT_HIGH_BOUND, (Int8)FormatFirstByte::NEGATIVE_FIXINT_LOW_BOUND))	// IsInBound args inverted because we expect a negative number
		{
			m_data.push_back(static_cast<byte>((Int8)data));
		}
		else
		{
			if (CanFit<UInt8>(data))
			{
				m_data.push_back((byte)FormatFirstByte::UINT_8);
				m_data.push_back(static_cast<byte>(data));
			}

			else if (CanFit<UInt16>(data))
			{
				const auto byteArray = ToBytes<UInt16, Endianness::BIG_ENDIAN>(data);

				InsertBytes<UInt16, FormatFirstByte::UINT_16>(byteArray);
			}

			else if (CanFit<UInt32>(data))
			{
				const auto byteArray = ToBytes<UInt32, Endianness::BIG_ENDIAN>(data);

				InsertBytes<UInt32, FormatFirstByte::UINT_32>(byteArray);
			}

			else if (CanFit<Int8>(data))
			{
				m_data.push_back((byte)FormatFirstByte::INT_8);
				m_data.push_back(static_cast<byte>(data));
			}

			else if (CanFit<Int16>(data))
			{
				const auto byteArray = ToBytes<Int16, Endianness::BIG_ENDIAN>(data);

				InsertBytes<Int16, FormatFirstByte::INT_16>(byteArray);
			}

			else if (CanFit<int>(data))
			{
				const auto byteArray = ToBytes<int, Endianness::BIG_ENDIAN>(data);

				InsertBytes<int, FormatFirstByte::INT_32>(byteArray);
			}

			else if (CanFit<UInt64>(data))
			{
				const auto byteArray = ToBytes<UInt64, Endianness::BIG_ENDIAN>(data);

				InsertBytes<UInt64, FormatFirstByte::UINT_64>(byteArray);
			}

			else
			{
				const auto byteArray = ToBytes<Int64, Endianness::BIG_ENDIAN>(data);

				InsertBytes<Int64, FormatFirstByte::INT_64>(byteArray);
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
		UNDEFINED,
		BOOL,
		POSITIVE_FIXUINT,
		NEGATIVE_FIXINT,
		UINT8,
		UINT16,
		UINT32,
		UINT64,
		INT8,
		INT16,
		INT32,
		INT64,
		TEXT,
		BYTES_BLOB,
		ARRAY_SIZE_HELPER
	};

	struct ItemLocation
	{
		size_t pos;
		size_t length = 0;
		ItemType type;
	};

	const std::vector<byte>& m_rawData;
	std::map<std::string, ItemLocation> m_parsedDataLocations;

public:
	MessagePackDeserializer(const std::vector<byte>& data) : m_rawData(data)
	{
	}

	void Parse()
	{
		Range<byte> range(m_rawData);

		ParseMap(range, "");
	}

	template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
	T GetValue(const std::string& key)
	{
		const auto itemDef = m_parsedDataLocations[key];
		Range<byte> bytesRange(m_rawData);
		bytesRange.Consume(itemDef.pos);

		switch (itemDef.type)
		{
		case ItemType::POSITIVE_FIXUINT:
			return (T)bytesRange[0];

		case ItemType::NEGATIVE_FIXINT:
			return (T)(Int8)bytesRange[0];

		case ItemType::UINT8:
			return ReadAndCheckInteger<UInt8, T>(bytesRange);

		case ItemType::UINT16:
			return ReadAndCheckInteger<UInt16, T>(bytesRange);

		case ItemType::UINT32:
			return ReadAndCheckInteger<UInt32, T>(bytesRange);

		case ItemType::UINT64:
			return ReadAndCheckInteger<UInt64, T>(bytesRange);

		case ItemType::INT8:
			return ReadAndCheckInteger<Int8, T>(bytesRange);

		case ItemType::INT16:
			return ReadAndCheckInteger<Int16, T>(bytesRange);

		case ItemType::INT32:
			return ReadAndCheckInteger<int, T>(bytesRange);

		case ItemType::INT64:
			return ReadAndCheckInteger<Int64, T>(bytesRange);

		default:
			throw std::exception("Can't convert the requested value");
		}
	}

	template<>
	bool GetValue(const std::string& key)
	{
		const auto itemDef = m_parsedDataLocations[key];
		if (itemDef.type != ItemType::BOOL)
			throw MessgePackInvalidFormat("The requested value is not of type BOOL");

		return (m_rawData[itemDef.pos] == (byte)FormatFirstByte::TRUE);
	}

	template<>
	std::string GetValue(const std::string& key)
	{
		const auto itemDef = m_parsedDataLocations[key];
		if (itemDef.type != ItemType::TEXT)
			throw MessgePackInvalidFormat("The requested value is not of type TEXT");

		return std::string(m_rawData.begin() + itemDef.pos, m_rawData.begin() + itemDef.pos + itemDef.length);
	}

	template<>
	std::vector<byte> GetValue(const std::string& key)
	{
		const auto itemDef = m_parsedDataLocations[key];
		if (itemDef.type != ItemType::BYTES_BLOB)
			throw MessgePackInvalidFormat("The requested value is not of type BYTES_BLOB");

		return std::vector<byte>(m_rawData.begin() + itemDef.pos, m_rawData.begin() + itemDef.pos + itemDef.length);
	}

	Range<byte> GetByteBlobRange(const std::string& key)
	{
		const auto itemDef = m_parsedDataLocations[key];
		if (itemDef.type != ItemType::BYTES_BLOB)
			throw MessgePackInvalidFormat("The requested value is not of type BYTES_BLOB");

		Range<byte> range(m_rawData, itemDef.length);
		range.Consume(itemDef.pos);

		return range;
	}

	template <typename T>
	std::optional<T> GetValueOptional(const std::string& key)
	{
		std::optional<T> output;

		if (m_parsedDataLocations.count(key) > 0)
			output = GetValue<T>(key);

		return output;
	}

	template <typename T>
	std::vector<T> GetArray(const std::string& key)
	{
		std::vector<T> output;
		const auto arrayDef = m_parsedDataLocations[key];

		if (arrayDef.type != ItemType::ARRAY_SIZE_HELPER)
			throw MessgePackInvalidFormat("The requested item is not an array");

		for (size_t i = 0; i < arrayDef.length; i++)
			output.push_back(GetValue<T>(key + "." + std::to_string(i)));

		return output;
	}

	template <typename T>
	std::optional<std::vector<T>> GetArrayOptional(const std::string& key)
	{
		std::optional<std::vector<T>> output;

		if (m_parsedDataLocations.count(key) > 0)
			output = GetArray<T>(key);

		return output;
	}

private:
	void ParseMap(Range<byte>& workingRange, const std::string& currentMapName)
	{
		const byte currentFirstByte = workingRange[0];
		workingRange.Consume(1);

		UInt32 nbItems;

		if (currentFirstByte == (byte)FormatFirstByte::MAP_16)
		{
			if (workingRange.Size() < sizeof(UInt16))
				throw MessgePackInvalidFormat("Byte sequence too short for UInt16");

			nbItems = (UInt32)ReadInteger<UInt16>(workingRange);
			workingRange.Consume(sizeof(UInt16));
		}

		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::FIX_MAP_LOW_BOUND, (byte)FormatFirstByte::FIX_MAP_HIGH_BOUND))
		{
			nbItems = static_cast<UInt32>((UInt8)currentFirstByte - (UInt8)FormatFirstByte::FIX_MAP_LOW_BOUND);
		}

		else if (currentFirstByte == (byte)FormatFirstByte::MAP_32)
		{
			if (workingRange.Size() < sizeof(UInt32))
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

	void ParseNextItem(Range<byte>& workingRange, const std::string& currentMapName, bool allowMap = true)
	{
		if (workingRange.Size() == 0)
			throw MessgePackInvalidFormat("Byte sequence too short");

		const byte currentFirstByte = workingRange[0];

		// Map
		if (allowMap && IsFirstByteMapIndex(currentFirstByte))
			ParseMap(workingRange, currentMapName);

		// Text
		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::FIX_STR_LOW_BOUND, (byte)FormatFirstByte::FIX_STR_HIGH_BOUND))
			ParseText(workingRange, currentMapName, false);

		else if (currentFirstByte == (byte)FormatFirstByte::STR_8)
			ParseText(workingRange, currentMapName, false);

		else if (currentFirstByte == (byte)FormatFirstByte::STR_16)
			ParseText(workingRange, currentMapName, false);

		else if (currentFirstByte == (byte)FormatFirstByte::STR_32)
			ParseText(workingRange, currentMapName, false);

		// bool
		else if (currentFirstByte == (byte)FormatFirstByte::FALSE || currentFirstByte == (byte)FormatFirstByte::TRUE)
		{
			ItemLocation item;
			item.pos = workingRange.GetCursorPosition();
			item.type = ItemType::BOOL;

			m_parsedDataLocations[currentMapName] = item;

			workingRange.Consume(1);
		}

		// Integer
		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::POSITIVE_FIXINT_LOW_BOUND, (byte)FormatFirstByte::POSITIVE_FIXINT_HIGH_BOUND))
		{
			ItemLocation item;
			item.pos = workingRange.GetCursorPosition();
			item.type = ItemType::POSITIVE_FIXUINT;

			m_parsedDataLocations[currentMapName] = item;

			workingRange.Consume(1);
		}

		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::NEGATIVE_FIXINT_HIGH_BOUND, (byte)FormatFirstByte::NEGATIVE_FIXINT_LOW_BOUND))
		{
			ItemLocation item;
			item.pos = workingRange.GetCursorPosition();
			item.type = ItemType::NEGATIVE_FIXINT;

			m_parsedDataLocations[currentMapName] = item;

			workingRange.Consume(1);
		}

		else if (currentFirstByte == (byte)FormatFirstByte::UINT_8)
			ParseInteger<UInt8>(workingRange, ItemType::UINT8, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::UINT_16)
			ParseInteger<UInt16>(workingRange, ItemType::UINT16, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::UINT_32)
			ParseInteger<UInt32>(workingRange, ItemType::UINT32, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::UINT_64)
			ParseInteger<UInt64>(workingRange, ItemType::UINT64, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::INT_8)
			ParseInteger<Int8>(workingRange, ItemType::INT8, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::INT_16)
			ParseInteger<Int16>(workingRange, ItemType::INT16, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::INT_32)
			ParseInteger<int>(workingRange, ItemType::INT32, currentMapName);

		else if (currentFirstByte == (byte)FormatFirstByte::INT_64)
			ParseInteger<Int64>(workingRange, ItemType::INT64, currentMapName);

		// Byte array
		else if (currentFirstByte == (byte)FormatFirstByte::BIN_8 || currentFirstByte == (byte)FormatFirstByte::BIN_16 || currentFirstByte == (byte)FormatFirstByte::BIN_32)
			ParseByteArray(workingRange, currentMapName);

		// Array
		else if (IsInBound(currentFirstByte, (byte)FormatFirstByte::FIXARRAY_LOW_BOUND, (byte)FormatFirstByte::FIXARRAY_HIGH_BOUND) || currentFirstByte == (byte)FormatFirstByte::ARRAY_16 || currentFirstByte == (byte)FormatFirstByte::ARRAY_32)
		{
			ParseArray(workingRange, currentMapName);
		}
	}

	std::string ParseText(Range<byte>& workingRange, const std::string& keyName, bool isKey)
	{
		const byte currentFirstByte = workingRange[0];
		UInt32 strLength = 0;
		workingRange.Consume(1);

		// Parse
		if (IsInBound(currentFirstByte, (byte)FormatFirstByte::FIX_STR_LOW_BOUND, (byte)FormatFirstByte::FIX_STR_HIGH_BOUND))
			strLength = static_cast<UInt8>(currentFirstByte - (byte)FormatFirstByte::FIX_STR_LOW_BOUND);

		else if (currentFirstByte == (byte)FormatFirstByte::STR_8)
			strLength = (UInt32)ReadAndConsumeInteger<UInt8>(workingRange);

		else if (currentFirstByte == (byte)FormatFirstByte::STR_16)
			strLength = (UInt32)ReadAndConsumeInteger<UInt16>(workingRange);

		else if (currentFirstByte == (byte)FormatFirstByte::STR_32)
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
			item.type = ItemType::TEXT;

			m_parsedDataLocations[keyName] = item;
			workingRange.Consume(strLength);
			return "";
		}
	}

	void ParseArray(Range<byte>& workingRange, const std::string& keyName)
	{
		const byte currentFirstByte = workingRange[0];
		workingRange.Consume(1);
		UInt32 nbItems = 0;

		// Parse
		if (IsInBound(currentFirstByte, (byte)FormatFirstByte::FIXARRAY_LOW_BOUND, (byte)FormatFirstByte::FIXARRAY_HIGH_BOUND))
			nbItems = static_cast<UInt8>(currentFirstByte - (byte)FormatFirstByte::FIXARRAY_LOW_BOUND);

		else if (currentFirstByte == (byte)FormatFirstByte::ARRAY_16)
			nbItems = (UInt32)ReadAndConsumeInteger<UInt16>(workingRange);

		else if (currentFirstByte == (byte)FormatFirstByte::ARRAY_32)
			nbItems = (UInt32)ReadAndConsumeInteger<UInt32>(workingRange);

		// Add helper
		ItemLocation item;
		item.type = ItemType::ARRAY_SIZE_HELPER;
		item.pos = 0;
		item.length = nbItems;
		m_parsedDataLocations[keyName] = item;

		// Parse the contained items
		for (size_t i = 0; i < nbItems; i++)
			ParseNextItem(workingRange, keyName + "." + std::to_string(i), false);
	}

	void ParseByteArray(Range<byte>& workingRange, const std::string& keyName)
	{
		const byte currentFirstByte = workingRange[0];
		UInt32 blobLenght = 0;

		if (currentFirstByte == (byte)FormatFirstByte::BIN_8)
			blobLenght = (UInt32)ReadAndConsumeInteger<UInt8>(workingRange);

		if (currentFirstByte == (byte)FormatFirstByte::BIN_16)
			blobLenght = (UInt32)ReadAndConsumeInteger<UInt16>(workingRange);

		if (currentFirstByte == (byte)FormatFirstByte::BIN_32)
			blobLenght = ReadAndConsumeInteger<UInt32>(workingRange);

		workingRange.Consume(1);

		ItemLocation item;
		item.length = blobLenght;
		item.pos = workingRange.GetCursorPosition();
		item.type = ItemType::BYTES_BLOB;

		m_parsedDataLocations[keyName] = item;
	}

	template <typename T>
	void ParseInteger(Range<byte>& workingRange, ItemType itemType, const std::string& keyName)
	{
		workingRange.Consume(1); // Consume the type specifier

		ItemLocation item;
		item.pos = workingRange.GetCursorPosition();
		item.type = itemType;

		m_parsedDataLocations[keyName] = item;
		workingRange.Consume(sizeof(T));
	}

	template <typename SOURCE_TYPE, typename TARGET_TYPE>
	TARGET_TYPE ReadAndCheckInteger(Range<byte>& bytesRange)
	{
		auto value = ReadInteger<SOURCE_TYPE>(bytesRange);

		if (CanFit<TARGET_TYPE>(value))
			return static_cast<TARGET_TYPE>(value);
		else
			throw std::out_of_range("The value do not fit into the specified type");
	}

	template <typename T>
	T ReadAndConsumeInteger(Range<byte>& bytesRange)
	{
		T value = ReadInteger<T>(bytesRange);
		bytesRange.Consume(sizeof(T));

		return value;
	}

	bool IsFirstByteMapIndex(byte nextByte)
	{
		return (nextByte == (byte)FormatFirstByte::MAP_16 || IsInBound(nextByte, (byte)FormatFirstByte::FIX_MAP_LOW_BOUND, (byte)FormatFirstByte::FIX_MAP_HIGH_BOUND) || nextByte == (byte)FormatFirstByte::MAP_32);
	}

	template <typename T>
	static T ReadInteger(Range<byte>& bytesRange)
	{
		std::array<byte, sizeof(T)> bytes;
		std::copy(bytesRange.begin(), bytesRange.begin() + sizeof(T), bytes.begin());

		return FromBytes<T, Endianness::BIG_ENDIAN>(bytes);
	}

	static std::string ReadText(Range<byte>& workingRange, UInt32 length)
	{
		if (length > workingRange.Size())
			throw MessgePackInvalidFormat("Byte sequence too short");

		return std::string(workingRange.begin(), workingRange.begin() + length);
	}
};