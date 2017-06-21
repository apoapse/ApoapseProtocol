#include "stdafx.h"
#include "GenericConnection.h"
#include "Common.h"
#include "MessagePack.hpp"
#include "NetworkPayload.h"

GenericConnection::GenericConnection(boost::asio::io_service& ioService) : TCPConnection(ioService)
{

}

GenericConnection::~GenericConnection()
{
	
}

bool GenericConnection::OnConnectedToServer()
{
	StartReading();
	return true;
}

bool GenericConnection::OnReceivedError(const boost::system::error_code& error)
{
	LOG << error.message() << LogSeverity::warning;

	return true;
}

void GenericConnection::StartReading()
{
	ReadSome(m_readBuffer, [this](size_t bytesTransferred) { OnReceivedHeaderData(bytesTransferred); });
}

void GenericConnection::OnReceivedHeaderData(size_t bytesTransferred)
{
	LOG << bytesTransferred;

	Range <std::array<byte, READ_BUFFER_SIZE>> range(m_readBuffer);
	auto headerInfo = ReadHeader(range);

	//std::vector<byte> data(m_readBuffer.begin(), m_readBuffer.begin() + bytesTransferred);
	LOG << std::string(m_readBuffer.begin(), m_readBuffer.begin() + bytesTransferred);
	
	StartReading();
}
