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
#include "TypeDefs.hpp"
#include "INetworkSender.h"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <deque>
#include <variant>
#include "NetworkPayload.h"
#include <boost/asio/ssl.hpp>
using namespace boost::asio;

class TCPConnection : public std::enable_shared_from_this<TCPConnection>, public INetworkSender
{
	using boostTCP = boost::asio::ip::tcp;
	using SSLSocket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

	friend class TCPServer;

	enum class Device
	{
		server,
		client,
	};

protected:
	std::unique_ptr<SSLSocket> m_socket;
	bool m_closeRequested = false;

private:
	boost::asio::io_context::strand m_strand;
	
	std::deque<std::variant<BytesWrapper, StrWrapper, std::shared_ptr<NetworkPayload>>> m_sendQueue;

public:
	using TCPConnection_ptr = std::shared_ptr<TCPConnection>;
	std::atomic<bool> m_isConnected = { false };

	TCPConnection(boost::asio::io_service& io_service, ssl::context& context);
	virtual ~TCPConnection() override = default;

	SSLSocket::lowest_layer_type& GetSocket()
	{
		return m_socket->lowest_layer();
	}

	boost::asio::ip::tcp::endpoint GetEndpoint() const;

	// INetworkSender
	void Connect(const std::string& adress, const UInt16 port);
	bool IsConnected() const;
	void Close() override;
	void RequestClose() override;

	void Send(BytesWrapper bytesPtr, TCPConnection* excludedConnection = nullptr) override;
	void Send(StrWrapper strPtr, TCPConnection* excludedConnection = nullptr) override;
	void Send(std::shared_ptr<NetworkPayload> payload, TCPConnection* excludedConnection = nullptr) override;

	std::string GetEndpointStr() const override;

private:
	void HandleConnectAsync(const boost::system::error_code& error, Device device);
	void HandleAcceptedAsync(const boost::system::error_code& error);

	void SendTLSHandshake(ssl::stream_base::handshake_type handshakeType);
	void HandleHandshake(const boost::system::error_code& error);

	void HandleReadInternal(const std::function<void(size_t)>& handler, const boost::system::error_code& error, size_t bytesTransferred);

	void InternalSend();
	void HandleWriteAsync(const boost::system::error_code& error, size_t bytesTransferred);

protected:
	void OnReceivedErrorInternal(const boost::system::error_code& error);
	
	template <typename T, typename FUNC>	// FUNC = std::function<void(size_t)>
	void ReadUntil(boost::asio::streambuf& streambuf, T delimiter, FUNC&& externalHandler)
	{
		auto handler = boost::bind(&TCPConnection::HandleReadInternal, shared_from_this(), externalHandler, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);

		boost::asio::async_read_until(GetSocket(), streambuf, delimiter, handler);
	}

	template <typename T, typename FUNC>	// FUNC = std::function<void(size_t)>
	void ReadSome(T& buffer, FUNC&& externalHandler)
	{
		auto handler = boost::bind(&TCPConnection::HandleReadInternal, shared_from_this(), externalHandler, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);

		m_socket->async_read_some(boost::asio::buffer(buffer), boost::asio::bind_executor(m_strand, handler));
	}

	virtual bool OnSocketConnectedInternal() = 0;
	virtual bool OnReceivedError(const boost::system::error_code& error) = 0;
	virtual void OnSendingSuccessful(size_t bytesTransferred) = 0;
	virtual void SetCustomTCPOptions() {}
};