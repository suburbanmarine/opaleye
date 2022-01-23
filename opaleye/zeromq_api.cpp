#include "zeromq_api.hpp"

zeromq_api::zeromq_api()
{

}
zeromq_api::~zeromq_api()
{

}

bool zeromq_api::init()
{
	m_context = std::make_shared<zmqpp::context>();
	m_context->set(zmqpp::context_option::io_threads,  4);
	m_context->set(zmqpp::context_option::max_sockets, 4);
	m_context->set(zmqpp::context_option::ipv6,        1);

	m_socket  = std::make_shared<zmqpp::socket>(*m_context, zmqpp::socket_type::reply);

	for(const std::string& str : m_ep)
	{
		m_socket->bind(str.c_str());
	}

	return true;
}
