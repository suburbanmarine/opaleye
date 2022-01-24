#include "zeromq_api_svr.hpp"

#include <zmq_addon.hpp>

#include <vector>

zeromq_api_svr::zeromq_api_svr()
{

}
zeromq_api_svr::~zeromq_api_svr()
{

}

bool zeromq_api_svr::init()
{
	m_context = std::make_shared<zmq::context_t>();
	m_pub_socket  = std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::pub);

	m_ep.push_back("tcp://127.0.0.1:50000");
	for(const std::string& str : m_ep)
	{
		m_pub_socket->bind(str.c_str());
	}

	m_api_pub_thread = std::make_shared<zeromq_api_svr_pub_thread>(m_pub_socket);
	m_api_pub_thread->launch();

	return true;
}

zeromq_api_svr_pub_thread::zeromq_api_svr_pub_thread(const std::shared_ptr<zmq::socket_t>& sock)
{
	m_socket = sock;
}

void zeromq_api_svr_pub_thread::work()
{
	while(!is_interrupted())
	{
		std::vector<zmq::const_buffer> msgs;
		msgs.push_back( zmq::str_buffer("/topic/foo") ); // Topic or URI
		msgs.push_back( zmq::str_buffer("text/plain") ); // MIME
		msgs.push_back( zmq::str_buffer("payload") );    // Payload
		zmq::send_multipart(*m_socket, msgs);

		wait_for_interruption(std::chrono::seconds(1));
	}
}
