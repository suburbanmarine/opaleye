#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <iostream>
#include <string>
#include <memory>

int main()
{

	std::shared_ptr<zmq::context_t> m_context = m_context = std::make_shared<zmq::context_t>();

	const std::string endpoint = "tcp://127.0.0.1:50000";
	std::shared_ptr<zmq::socket_t> m_socket  = std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::sub);
	m_socket->connect(endpoint);
    m_socket->set(zmq::sockopt::subscribe, "/topic/foo");

	std::vector<zmq::message_t> rcv_msgs;
	for(size_t i = 0; i < 10; i++)
	{
		zmq::recv_multipart(*m_socket, std::back_inserter(rcv_msgs));
	}

	return 0;
}
