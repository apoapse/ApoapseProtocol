#pragma once
#include <memory>
#include "TypeDefs.hpp"
#include "INetworkSender.h"
#include <boost\asio.hpp>
#include <boost\bind.hpp>
#include <deque>
#include <variant>
#include "NetworkPayload.h"

class TCPConnection : public std::enable_shared_from_this<TCPConnection>, public INetworkSender
{
	using boostTCP = boost::asio::ip::tcp;
	friend class TCPServer;

private:
	boostTCP::socket m_socket;
	std::atomic<bool> m_isConnected = { false };
	//boost::asio::io_service::strand m_writeStrand;

	std::deque<std::variant<BytesWrapper, StrWrapper, std::unique_ptr<NetworkPayload>>> m_sendQueue;

public:
	using TCPConnection_ptr = std::shared_ptr<TCPConnection>;

	TCPConnection(boost::asio::io_service& io_service);
	virtual ~TCPConnection() override = default;

	boostTCP::socket& GetSocket()
	{
		return m_socket;
	}

	boost::asio::ip::tcp::endpoint GetEndpoint() const;

	// INetworkSender
	void Connect(const std::string& adress, const UInt16 port);
	bool IsConnected() const;
	void Close() override;

	void Send(BytesWrapper bytesPtr, TCPConnection* excludedConnection = nullptr) override;
	void Send(StrWrapper strPtr, TCPConnection* excludedConnection = nullptr) override;
	void Send(std::unique_ptr<NetworkPayload> payload, TCPConnection* excludedConnection = nullptr) override;

	std::string GetEndpointStr() const override;

private:
	void HandleConnectAsync(const boost::system::error_code& error);
	void HandleAcceptedAsync(const boost::system::error_code& error);
	void OnReceivedErrorInternal(const boost::system::error_code& error);

	void HandleReadInternal(const std::function<void(size_t)>& handler, const boost::system::error_code& error, size_t bytesTransferred);

	void InternalSend();
	void HandleWriteAsync(const boost::system::error_code& error, size_t bytesTransferred);

protected:
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

		m_socket.async_read_some(boost::asio::buffer(buffer), handler);
	}

	virtual bool OnConnectedToServerInternal() = 0;
	virtual bool OnReceivedError(const boost::system::error_code& error) = 0;
	virtual void OnSendingSuccessful(size_t bytesTransferred) = 0;
};