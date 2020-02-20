// ----------------------------------------------------------------------------
// Copyright (C) 2020 Apoapse
// Copyright (C) 2020 Guillaume Puyal
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0
//
// For more information visit https://github.com/apoapse/
// And https://apoapse.space/
// ----------------------------------------------------------------------------

#pragma once
#include <memory>
#include <string>
#include <vector>
#include "TypeDefs.hpp"
class TCPConnection;

using StrWrapper = std::shared_ptr<std::string>;
using BytesWrapper = std::shared_ptr<std::vector<byte>>;

struct INetworkSender
{
	virtual ~INetworkSender() = default;

	//************************************
	// Method:    INetworkSender::Send - Send a byte array to a connection or a list of connections (in the case of an user for instance)
	// Access:    virtual public 
	// Returns:   void
	// Parameter: std::shared_ptr<std::vector<byte>> bytesPtr - Byte array shared pointer. A smart pointer is used because TCPConnection need ownership and persistance of the data.
	// Parameter: GenericConnection * excludedConnection - In case the implementation of this function send to multiple connections, this argument can be used to exclude one particular connection where the data will not be sent.
	//************************************
	virtual void Send(BytesWrapper bytesPtr, TCPConnection* excludedConnection = nullptr) = 0;

	//************************************
	// Method:    INetworkSender::Send - Send a string to a connection or a list of connections (in the case of an user for instance)
	// Access:    virtual public 
	// Returns:   void
	// Parameter: std::unique_ptr<std::string> strPtr - String unique pointer. A smart pointer is used because TCPConnection need ownership and persistance of the data.
	// Parameter: GenericConnection * excludedConnection - In case the implementation of this function send to multiple connections, this argument can be used to exclude one particular connection where the data will not be sent.
	//************************************
	virtual void Send(StrWrapper strPtr, TCPConnection* excludedConnection = nullptr) = 0;

	virtual void Send(std::shared_ptr<class NetworkPayload> data, TCPConnection* excludedConnection = nullptr) = 0;

	//virtual boost::asio::ip::tcp::endpoint GetEndpoint() const = 0;
	virtual std::string GetEndpointStr() const = 0;

	// Close immediately the connection
	virtual void Close() = 0;

	// Close the connection as soon as cmd processing is completed
	virtual void RequestClose() = 0;
};