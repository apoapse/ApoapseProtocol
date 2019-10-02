#pragma once
#include "TypeDefs.hpp"

struct ICustomDataType
{
	ICustomDataType() = default;
	virtual ~ICustomDataType() = default;

	virtual std::vector<byte> GetBytes() const
	{
		throw std::runtime_error("ICustomDataType members should not be called directly");
	}

	virtual std::string GetStr() const
	{
		throw std::runtime_error("ICustomDataType members should not be called directly");
	}
};