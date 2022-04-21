#include "zeromq_api_svr.hpp"

#include <zmq_addon.hpp>

#include <spdlog/spdlog.h>

#include <vector>


zeromq_api_svr::zeromq_api_svr()
{

}
zeromq_api_svr::~zeromq_api_svr()
{
	stop();
}

bool zeromq_api_svr::init(const std::list<std::string>& ep)
{
	if(m_context || m_pub_socket)
	{
		return false;
	}

	m_context = std::make_shared<zmq::context_t>();
	if( ! m_context )
	{
		SPDLOG_ERROR("Could not allocate context"); 
		return false;
	}
	m_pub_socket  = std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::pub);
	if( ! m_pub_socket )
	{
		SPDLOG_ERROR("Could not allocate socket"); 
		return false;
	}

	m_pub_socket->set(zmq::sockopt::metadata, zmq::str_buffer("X-Opaleye-api:1.0"));
	// m_pub_socket->set(zmq::sockopt::bindtodevice, zmq::str_buffer("X-Opaleye-api:1.0"));
    m_pub_socket->set(zmq::sockopt::connect_timeout, 5*1000);
	m_pub_socket->set(zmq::sockopt::ipv6, 0);
	m_pub_socket->set(zmq::sockopt::linger, 0);
	m_pub_socket->set(zmq::sockopt::maxmsgsize, int64_t(50LL*1024LL*1024LL));
	m_pub_socket->set(zmq::sockopt::multicast_hops, 1);
	m_pub_socket->set(zmq::sockopt::rcvbuf, 100*1024*1024);
	m_pub_socket->set(zmq::sockopt::rcvtimeo, 10*1000);
	m_pub_socket->set(zmq::sockopt::sndbuf, 100*1024*1024);
	m_pub_socket->set(zmq::sockopt::sndtimeo, 10*1000);
	m_pub_socket->set(zmq::sockopt::rcvhwm, 10);
	m_pub_socket->set(zmq::sockopt::sndhwm, 10);
	// m_socket->set(zmq::sockopt::tos, 0);

	m_ep = ep;
	for(const std::string& str : m_ep)
	{
		SPDLOG_INFO("Binding to {:s}", str);
		try
		{
			m_pub_socket->bind(str.c_str());
		}
		catch(const zmq::error_t& e)
		{
			SPDLOG_ERROR("ZMQ bind failed: {:d}: {:s}", e.num(), e.what());
			stop();
			return false;
		}
	}

	m_api_pub_thread = std::make_shared<zeromq_api_svr_pub_thread>(m_pub_socket);
	m_api_pub_thread->launch();

	return true;
}

bool zeromq_api_svr::stop()
{
	if(m_pub_socket)
	{
		m_pub_socket->close();
		m_pub_socket.reset();
	}
	if(m_context)
	{
		m_context->shutdown();
		m_context->close();
		m_context.reset();
	}

	return true;
}

bool zeromq_api_svr::send(const std::string_view& topic, const std::string_view& header, const std::string_view& payload)
{
	std::lock_guard<std::mutex> lock(m_pub_socket_mutex);

	std::array<zmq::const_buffer, 3> msgs;
	msgs[0] = zmq::const_buffer(topic.data(),   topic.size());   // Topic or URI
	msgs[1] = zmq::const_buffer(header.data(),  header.size());  // MIME
	msgs[2] = zmq::const_buffer(payload.data(), payload.size()); // Payload

	bool func_ret = true;

	zmq::send_result_t res;
	try
	{
		 // res = zmq::send_multipart(*m_pub_socket, msgs, zmq::send_flags::dontwait);
		res = zmq::send_multipart(*m_pub_socket, msgs, zmq::send_flags::none);
	}
	catch(const zmq::error_t& e)
	{
		SPDLOG_ERROR("zmq::send_multipart got error: {:s}", e.what());
		func_ret = false;
	}

	if( ! res )
	{
		SPDLOG_WARN("zmq::send_multipart did nothing, errno: {:d}", errno);
		func_ret = false;
	}

	return func_ret;
}

zeromq_api_svr_pub_thread::zeromq_api_svr_pub_thread(const std::shared_ptr<zmq::socket_t>& sock)
{
	m_socket = sock;
}

void zeromq_api_svr_pub_thread::work()
{
	while(!is_interrupted())
	{
		// std::vector<zmq::const_buffer> msgs;
		// msgs.push_back( zmq::str_buffer("/topic/foo") ); // Topic or URI
		// msgs.push_back( zmq::str_buffer("text/plain") ); // MIME
		// msgs.push_back( zmq::str_buffer("payload") );    // Payload
		// zmq::send_multipart(*m_socket, msgs);

		wait_for_interruption(std::chrono::seconds(1));
	}
}
