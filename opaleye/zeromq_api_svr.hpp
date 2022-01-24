#pragma once

#include <thread_base.hpp>

#include <zmq.hpp>

#include <memory>
#include <list>


// part 1 - topic
// part 2 - Headers
			// maybe HTTP, eg Content-Type: text/plain\r\nCache-Control: no-cache,no-store\r\nLast-Modified: <>
			// maybe a json blob
			// maybe just a mime type, text/plain
// part 3 - data blob

// /api/v1/cameras/<camera-name>/live/full

class zeromq_api_svr_pub_thread : public thread_base
{
public:
	zeromq_api_svr_pub_thread(const std::shared_ptr<zmq::socket_t>& sock);

	void work();

protected:
	std::shared_ptr<zmq::socket_t>  m_socket;
};

class zeromq_api_svr
{
public:
	zeromq_api_svr();
	~zeromq_api_svr();

	bool init(const std::list<std::string>& ep);

	bool stop();

	bool send(const std::string_view& topic, const std::string_view& header, const std::string_view& payload);

protected:

	std::shared_ptr<zeromq_api_svr_pub_thread> m_api_pub_thread;

	std::shared_ptr<zmq::context_t> m_context;

	//publish socket
	std::list<std::string> m_ep;
	std::shared_ptr<zmq::socket_t>  m_pub_socket;

	// JSON RPC bridge
	// std::shared_ptr<zmq::socket_t>  m_json_rpc_reply_socket;
};

