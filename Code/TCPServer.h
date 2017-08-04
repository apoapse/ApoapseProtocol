#pragma once
#include <boost\asio.hpp>
#include <memory>
#include "TCPConnection.h"

class TCPServer
{
	using boostTCP = boost::asio::ip::tcp;

	boost::asio::io_service& m_ioservice;
	std::unique_ptr<boostTCP::acceptor> m_acceptor;
	UInt16 m_port;

public:
	enum class Protocol
	{
		ip_v4,
		ip_v6
	};

	TCPServer(boost::asio::io_service& io_service, UInt16 requestedPort, Protocol ipProtocol = Protocol::ip_v4);
	virtual ~TCPServer();

	void Close();

	template <typename T_CONNECTION, typename... T_Args>
	void StartAccept(T_Args&&... args)
	{
		TCPConnection::TCPConnection_ptr newConnection = std::make_shared<T_CONNECTION>(m_ioservice, std::forward<T_Args>(args)...);	//TODO: HAVE A FLAG TO SPECIFY THAT THIS CONNECTION IS THE SERVER ITSELF?

		auto handler = boost::bind(&TCPServer::HandleAcceptAsync<T_CONNECTION, T_Args&...>, this, newConnection, boost::asio::placeholders::error, args...);
		m_acceptor->async_accept(newConnection->GetSocket(), handler);
	}

private:
	template <typename T_CONNECTION, typename... T_Args>
	void HandleAcceptAsync(TCPConnection::TCPConnection_ptr tcpConnection, const boost::system::error_code& error, T_Args&&... args)
	{
		tcpConnection->HandleAcceptedAsync(error);

		StartAccept<T_CONNECTION>(std::forward<T_Args>(args)...);	// Once a connection is accepted, we call back the acceptor to handle the next connection
	}
};