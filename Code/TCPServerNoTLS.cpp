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

#include "stdafx.h"
#include "Common.h"
#include "TCPServerNoTLS.h"

TCPServerNoTLS::TCPServerNoTLS(boost::asio::io_service& io_service, UInt16 port, Protocol ipProtocol /*= Protocol::IP_v4*/)
	: m_acceptor(std::make_unique<boostTCP::acceptor>(io_service))
	, m_ioservice(io_service)
{
	boost::system::error_code error;
	boostTCP::endpoint endpoint;

	if (ipProtocol == Protocol::ip_v4)
		endpoint = boostTCP::endpoint(boostTCP::v4(), port);

	else if (ipProtocol == Protocol::ip_v6)
		endpoint = boostTCP::endpoint(boostTCP::v6(), port);

	m_acceptor->open(endpoint.protocol(), error);
	m_acceptor->set_option(boostTCP::acceptor::reuse_address(true));

	m_acceptor->bind(endpoint, error);

	if (!error)
	{
		m_port = endpoint.port();

		LOG << "TCP Server started at " << endpoint.address() << ", port " << endpoint.port() << " protocol " << endpoint.protocol().protocol();

		m_acceptor->listen();
	}
	else
		throw error;
}

TCPServerNoTLS::~TCPServerNoTLS()
{
	LOG << "TCP Server on port " << m_port << " stopped";
}

void TCPServerNoTLS::Close()
{
	m_acceptor->close();
}