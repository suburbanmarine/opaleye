#pragma once

#include <thread_base.hpp>

#include <zmq.hpp>

#include <memory>
#include <list>

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

	bool init();

protected:

	std::shared_ptr<zeromq_api_svr_pub_thread> m_api_pub_thread;

	std::shared_ptr<zmq::context_t> m_context;

	//publish socket
	std::list<std::string> m_ep;
	std::shared_ptr<zmq::socket_t>  m_pub_socket;

	// JSON RPC bridge
	// std::shared_ptr<zmq::socket_t>  m_json_rpc_reply_socket;
};

