#include "stdafx.h"
#include "Common.h"
#include "TCPConnection.h"
#include "ThreadUtils.h"
#include <boost/exception/all.hpp>

TCPConnection::TCPConnection(boost::asio::io_service& io_service, ssl::context& context) : m_strand(io_service)
{
	m_socket = std::make_unique<SSLSocket>(io_service, context);
}

void TCPConnection::Connect(const std::string& ipAddress, const UInt16 port)
{
	ASSERT(!IsConnected());
	auto destination = boostTCP::endpoint(boost::asio::ip::address::from_string(ipAddress), port);

	GetSocket().async_connect(destination, boost::bind(&TCPConnection::HandleConnectAsync, shared_from_this(), boost::asio::placeholders::error, Device::client));
}

bool TCPConnection::IsConnected() const
{
	return m_isConnected;
}

void TCPConnection::Close()
{
	LOG << "Manual closing of the TCP connection socket";

	m_isConnected = false;

	auto self(shared_from_this());
	m_strand.post([this, self]
	{
		try
		{
			GetSocket().shutdown(boostTCP::socket::shutdown_both);
			GetSocket().close();
		}
		catch (const boost::exception& e)
        {
			LOG << LogSeverity::error << boost::diagnostic_information(e);
        }
	});
}

void TCPConnection::RequestClose()
{
	m_closeRequested = true;
}

boost::asio::ip::tcp::endpoint TCPConnection::GetEndpoint() const
{
	try
	{
		return m_socket->lowest_layer().remote_endpoint();
	}
	catch (const std::exception&)
	{
		ASSERT_MSG(false, "TCPConnection::GetEndpoint() is probably called too early");
		return boost::asio::ip::tcp::endpoint();
	}
}

void TCPConnection::HandleConnectAsync(const boost::system::error_code& error, Device device)
{
	if (error)
	{
		OnReceivedErrorInternal(error);
	}
	else
	{
		SetCustomTCPOptions();
		
		if (device == Device::server)
		{
			SendTLSHandshake(ssl::stream_base::handshake_type::server);
		}
		else
		{
			SendTLSHandshake(ssl::stream_base::handshake_type::client);
		}
	}
}

void TCPConnection::HandleAcceptedAsync(const boost::system::error_code& error)
{
	LOG_DEBUG << "TCPConnection accepted from " << GetEndpoint();
	HandleConnectAsync(error, Device::server);
}

void TCPConnection::OnReceivedErrorInternal(const boost::system::error_code& error)
{
	if (!this->OnReceivedError(error))
		Close();
}

void TCPConnection::SendTLSHandshake(ssl::stream_base::handshake_type handshakeType)
{
	m_socket->async_handshake(handshakeType, boost::bind(&TCPConnection::HandleHandshake, shared_from_this(), boost::asio::placeholders::error));
}

void TCPConnection::HandleHandshake(const boost::system::error_code& error)
{
	if (error)
	{
		LOG << LogSeverity::error << "TLS Handshake failed";
		Close();
	}

	this->OnSocketConnectedInternal();
	m_isConnected = true;
}

void TCPConnection::HandleReadInternal(const std::function<void(size_t)>& handler, const boost::system::error_code& error, size_t bytesTransferred)
{
	if (!IsConnected())
		return;

	if (error && error != boost::asio::error::not_found)
	{
		OnReceivedErrorInternal(error);
		return;
	}

	handler(bytesTransferred);
}

void TCPConnection::Send(BytesWrapper bytesPtr, TCPConnection* excludedConnection/* = nullptr*/)
{
	if (excludedConnection == this)
		return;

	const bool isWriteInProgress = !m_sendQueue.empty();
	m_sendQueue.emplace_back(bytesPtr);

	if (!isWriteInProgress)
		InternalSend();

	// #TODO #THREADING Use the following code to make the Send operation thread safe
	// m_socket.get_io_service().post(m_writeStrand.wrap([bytesPtr = std::move(bytesPtr), this]() mutable
	// 	{
	// 
	// 	}));
}

void TCPConnection::Send(StrWrapper strPtr, TCPConnection* excludedConnection/* = nullptr*/)
{
	if (excludedConnection == this)
		return;

	const bool isWriteInProgress = !m_sendQueue.empty();
	m_sendQueue.emplace_back(std::move(strPtr));

	if (!isWriteInProgress)
		InternalSend();

	// #TODO #THREADING Use the following code to make the Send operation thread safe
	// 	m_socket.get_io_service().post(m_writeStrand.wrap([strPtr = std::move(strPtr), this]() mutable
	// 	{
	// 
	// 	}));
}

void TCPConnection::Send(std::shared_ptr<NetworkPayload> payload, TCPConnection* excludedConnection /*= nullptr*/)
{
	if (excludedConnection == this)
		return;

	const bool isWriteInProgress = !m_sendQueue.empty();
	m_sendQueue.emplace_back(payload);

	if (!isWriteInProgress)
		InternalSend();

	// #TODO #THREADING Use the following code to make the Send operation thread safe
	// m_socket.get_io_service().post(m_writeStrand.wrap([bytesPtr = std::move(bytesPtr), this]() mutable
	// 	{
	// 
	// 	}));
}

std::string TCPConnection::GetEndpointStr() const
{
	return GetEndpoint().address().to_string() + ":" + std::to_string(GetEndpoint().port());
}

void TCPConnection::InternalSend()
{
	const auto& item = m_sendQueue.front();
	auto handler = boost::bind(&TCPConnection::HandleWriteAsync, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);

	if (std::holds_alternative<std::shared_ptr<NetworkPayload>>(item))
	{	
		m_socket->async_write_some(boost::asio::buffer(std::get<std::shared_ptr<NetworkPayload>>(item)->GetRawData()), boost::asio::bind_executor(m_strand, handler));
		//boost::asio::async_write(GetSocket(), boost::asio::buffer(std::get<std::shared_ptr<NetworkPayload>>(item)->GetRawData()), handler);
	}
	else if (std::holds_alternative<StrWrapper>(item))
	{
		m_socket->async_write_some(boost::asio::buffer(*std::get<StrWrapper>(item)), boost::asio::bind_executor(m_strand, handler));
		//boost::asio::async_write(GetSocket(), boost::asio::buffer(*std::get<StrWrapper>(item)), handler);
	}
	else
	{
		//boost::asio::async_write(GetSocket(), boost::asio::buffer(*std::get<BytesWrapper>(item)), handler);
		m_socket->async_write_some(boost::asio::buffer(*std::get<BytesWrapper>(item)), boost::asio::bind_executor(m_strand, handler));
	}
	
}

void TCPConnection::HandleWriteAsync(const boost::system::error_code& error, size_t bytesTransferred)
{
	if (!IsConnected())
		return;

	if (!error)
	{
		const auto& item = m_sendQueue.front();

		size_t itemRealSize = 0;

		if (std::holds_alternative<std::shared_ptr<NetworkPayload>>(item))
		{
			auto& payload = std::get<std::shared_ptr<NetworkPayload>>(item);
			itemRealSize = (NetworkPayload::headerLength + payload->headerInfo->payloadLength);
		}
		else if (std::holds_alternative<StrWrapper>(item))
		{
			itemRealSize = std::get<StrWrapper>(item)->length();
		}
		else
		{
			itemRealSize = std::get<BytesWrapper>(item)->size();
		}
		ASSERT(itemRealSize > 0);

		if (itemRealSize == bytesTransferred)
		{
			LOG_DEBUG << bytesTransferred << " bytes has been sent successfully to " << GetEndpoint() << " from thread " << ThreadUtils::GetThreadName();
			OnSendingSuccessful(bytesTransferred);
		}
		else
		{
			LOG << itemRealSize << " bytes expected to be sent to " << GetEndpoint() << " but only " << bytesTransferred << " bytes has been transferred" << LogSeverity::error;
			Close();
		}
	}
	else
		OnReceivedErrorInternal(error);

	//if (!m_sendQueue.empty())
		m_sendQueue.pop_front();

	if (!m_sendQueue.empty())
		InternalSend();
}