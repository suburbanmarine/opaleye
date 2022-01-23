#pragma once

#include <zmqpp/zmqpp.hpp>

#include <memory>

class zeromq_api
{
public:
	zeromq_api();
	~zeromq_api();

	bool init();

protected:
	std::shared_ptr<zmqpp::context> m_context;

	std::list<std::string> m_ep;
	std::shared_ptr<zmqpp::socket>  m_socket;
};
