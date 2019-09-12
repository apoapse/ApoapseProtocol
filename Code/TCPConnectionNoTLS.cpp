#include "stdafx.h"
#include "Common.h"
#include "TCPConnectionNoTLS.h"
#include "ThreadUtils.h"

TCPConnectionNoTLS::TCPConnectionNoTLS(boost::asio::io_service& io_service)
	//, m_writeStrand(io_service)
{
	m_socket = std::make_unique<Socket>(io_service);
}

void TCPConnectionNoTLS::Connect(const std::string& ipAddress, const UInt16 port)
{
	ASSERT(!IsConnected());
	auto destination = boostTCP::endpoint(boost::asio::ip::address::from_string(ipAddress), port);

	GetSocket().async_connect(destination, boost::bind(&TCPConnectionNoTLS::HandleConnectAsync, shared_from_this(), boost::asio::placeholders::error, Device::client));
}

bool TCPConnectionNoTLS::IsConnected() const
{
	return m_isConnected;
}

void TCPConnectionNoTLS::Close()
{
	LOG << "Manual closing of the TCP connection socket " << GetEndpoint();

	m_isConnected = false;

	GetSocket().shutdown(boostTCP::socket::shutdown_both);
	GetSocket().close();
	//GetSocket().release();
}

void TCPConnectionNoTLS::RequestClose()
{
	m_closeRequested = true;
}

boost::asio::ip::tcp::endpoint TCPConnectionNoTLS::GetEndpoint() const
{
	try
	{
		return m_socket->lowest_layer().remote_endpoint();
	}
	catch (const std::exception&)
	{
		ASSERT_MSG(false, "TCPConnectionNoTLS::GetEndpoint() is probably called too early");
		return boost::asio::ip::tcp::endpoint();
	}
}

void TCPConnectionNoTLS::HandleConnectAsync(const boost::system::error_code& error, Device device)
{
	if (error)
	{
		OnReceivedErrorInternal(error);
	}
	else
	{
		m_isConnected = true;
		this->OnSocketConnectedInternal();
	}
}

void TCPConnectionNoTLS::HandleAcceptedAsync(const boost::system::error_code& error)
{
	LOG_DEBUG << "TCPConnectionNoTLS accepted from " << GetEndpoint();
	HandleConnectAsync(error, Device::server);
}

void TCPConnectionNoTLS::OnReceivedErrorInternal(const boost::system::error_code& error)
{
	if (!this->OnReceivedError(error))
		Close();
}

void TCPConnectionNoTLS::HandleReadInternal(const std::function<void(size_t)>& handler, const boost::system::error_code& error, size_t bytesTransferred)
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

void TCPConnectionNoTLS::Send(BytesWrapper bytesPtr, TCPConnection* excludedConnection/* = nullptr*/)
{
	//if (excludedConnection == this)
	//	return;

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

void TCPConnectionNoTLS::Send(StrWrapper strPtr, TCPConnection* excludedConnection/* = nullptr*/)
{
	//if (excludedConnection == this)
	//	return;

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

void TCPConnectionNoTLS::Send(std::shared_ptr<NetworkPayload> payload, TCPConnection* excludedConnection /*= nullptr*/)
{
	//if (excludedConnection == this)
	//	return;

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

std::string TCPConnectionNoTLS::GetEndpointStr() const
{
	return GetEndpoint().address().to_string() + ":" + std::to_string(GetEndpoint().port());
}

void TCPConnectionNoTLS::InternalSend()
{
	const auto& item = m_sendQueue.front();
	auto handler = boost::bind(&TCPConnectionNoTLS::HandleWriteAsync, shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred);

	if (std::holds_alternative<std::shared_ptr<NetworkPayload>>(item))
	{	
		m_socket->async_write_some(boost::asio::buffer(std::get<std::shared_ptr<NetworkPayload>>(item)->GetRawData()), handler);
		//boost::asio::async_write(GetSocket(), boost::asio::buffer(std::get<std::shared_ptr<NetworkPayload>>(item)->GetRawData()), handler);
	}
	else if (std::holds_alternative<StrWrapper>(item))
	{
		m_socket->async_write_some(boost::asio::buffer(*std::get<StrWrapper>(item)), handler);
		//boost::asio::async_write(GetSocket(), boost::asio::buffer(*std::get<StrWrapper>(item)), handler);
	}
	else
	{
		//boost::asio::async_write(GetSocket(), boost::asio::buffer(*std::get<BytesWrapper>(item)), handler);
		m_socket->async_write_some(boost::asio::buffer(*std::get<BytesWrapper>(item)), handler);
	}
	
}

void TCPConnectionNoTLS::HandleWriteAsync(const boost::system::error_code& error, size_t bytesTransferred)
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