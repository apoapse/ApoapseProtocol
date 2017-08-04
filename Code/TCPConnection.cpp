#include "stdafx.h"
#include "Common.h"
#include "TCPConnection.h"

TCPConnection::TCPConnection(boost::asio::io_service& io_service)
	: m_socket(io_service)
	//, m_writeStrand(io_service)
{
}

void TCPConnection::Connect(const std::string& ipAddress, const UInt16 port)
{
	ASSERT(!IsConnected());
	auto destination = boostTCP::endpoint(boost::asio::ip::address::from_string(ipAddress), port);

	m_socket.async_connect(destination, boost::bind(&TCPConnection::HandleConnectAsync, shared_from_this(), boost::asio::placeholders::error));
}

bool TCPConnection::IsConnected() const
{
	return m_isConnected;
}

void TCPConnection::Close()
{
	LOG << "Manual closing of the TCP connection socket " << GetEndpoint();

	m_isConnected = false;
	m_socket.close();
}

boost::asio::ip::tcp::endpoint TCPConnection::GetEndpoint() const
{
	try
	{
		return m_socket.remote_endpoint();
	}
	catch (const std::exception&)
	{
		ASSERT_MSG(false, "TCPConnection::GetEndpoint() is probably called too early");
		return boost::asio::ip::tcp::endpoint();
	}
}

void TCPConnection::HandleConnectAsync(const boost::system::error_code& error)
{
	if (error)
		OnReceivedErrorInternal(error);
	else
	{
		this->OnConnectedToServerInternal();
		m_isConnected = true;
	}
}

void TCPConnection::HandleAcceptedAsync(const boost::system::error_code& error)
{
	LOG_DEBUG << "TCPConnection accepted from " << GetEndpoint();
	HandleConnectAsync(error);
}

void TCPConnection::OnReceivedErrorInternal(const boost::system::error_code& error)
{
	if (!this->OnReceivedError(error))
		Close();
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
	ASSERT(excludedConnection == nullptr);

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
	ASSERT(excludedConnection == nullptr);

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

void TCPConnection::InternalSend()
{
	const auto& item = m_sendQueue.front();

	ASSERT_MSG(std::holds_alternative<StrWrapper>(item) || std::holds_alternative<BytesWrapper>(item), "Unsupported input type");

	if (std::holds_alternative<StrWrapper>(item))
	{
		auto handler = boost::bind(&TCPConnection::HandleWriteAsync, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
		boost::asio::async_write(m_socket, boost::asio::buffer(*std::get<StrWrapper>(item)), handler);
	}
	else
	{
		auto handler = boost::bind(&TCPConnection::HandleWriteAsync, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);
		boost::asio::async_write(m_socket, boost::asio::buffer(*std::get<BytesWrapper>(item)), handler);
	}
}

void TCPConnection::HandleWriteAsync(const boost::system::error_code& error, size_t bytesTransferred)
{
	if (!IsConnected())
		return;

	if (!error)
	{
		const auto& item = m_sendQueue.front();
		const size_t itemRealSize = std::holds_alternative<StrWrapper>(item) ? std::get<StrWrapper>(item)->length() : std::get<BytesWrapper>(item)->size();

		if (itemRealSize == bytesTransferred)
		{
			LOG << bytesTransferred << " bytes has been sent successfully to " << GetEndpoint() << LogSeverity::debug;
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

	m_sendQueue.pop_front();

	if (!m_sendQueue.empty())
		InternalSend();
}