#pragma once

#include "zmq.hpp"

#include <iostream>

int main()
{

	std::shared_ptr<zmq::context_t> m_context = m_context = std::make_shared<zmq::context_t>();

	const string endpoint = "tcp://localhost:5555";
	std::shared_ptr<zmq::socket_t> m_socket  = std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::sub);
	m_socket->connect(endpoint);
	m_socket->subscribe("/test")

	std::vector<zmq::message_t> rcv_msgs;
	for(size_t i = 0; i < 10; i++)
	{
		zmq::recv_multipart(*m_socket, std::back_inserter(rcv_msgs));
	}

	return 0;
}
