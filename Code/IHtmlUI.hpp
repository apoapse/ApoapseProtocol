#pragma once
#include "TypeDefs.hpp"

struct IHtmlUI
{
	virtual ~IHtmlUI() = default;

	virtual void SendSignal(const std::string& name, const std::string& data) = 0;
	virtual void UpdateStatusBar(const std::string& msg, bool isError = false) = 0;
};