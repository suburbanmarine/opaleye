#pragma once

#include <opaleye-util/thread_base.hpp>

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
	~zeromq_api_svr_pub_thread() override;

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

	///
	/// This is globally serializing - pub sockets are not thread safe
	/// RADIO/DISH is thread safe but does not support multipart messages - we want multipart messages so topic is in own segment
	/// Could make socket per topic that needs to be in own async send domain
	///
	bool send(const std::string_view& topic, const std::string_view& header, const std::string_view& payload);

	// bool async_send(const std::string_view& topic, const std::string_view& header, const std::string_view& payload);

protected:

	std::shared_ptr<zeromq_api_svr_pub_thread> m_api_pub_thread;

	std::shared_ptr<zmq::context_t> m_context;

	//publish socket
	std::mutex m_pub_socket_mutex;
	std::list<std::string> m_ep;
	std::shared_ptr<zmq::socket_t>  m_pub_socket;

	// JSON RPC bridge
	// std::shared_ptr<zmq::socket_t>  m_json_rpc_reply_socket;
};

