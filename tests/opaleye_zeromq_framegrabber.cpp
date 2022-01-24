#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <iostream>
#include <string>
#include <memory>

int main()
{

	std::shared_ptr<zmq::context_t> m_context = std::make_shared<zmq::context_t>();

	const std::string endpoint = "tcp://127.0.0.1:50000";
	std::shared_ptr<zmq::socket_t> m_socket  = std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::sub);
    
    m_socket->set(zmq::sockopt::subscribe, "/api/v1/cameras/cam0/live/full");
    m_socket->set(zmq::sockopt::connect_timeout, 5*1000);
	m_socket->set(zmq::sockopt::ipv6, 0);
	m_socket->set(zmq::sockopt::linger, 0);
	m_socket->set(zmq::sockopt::maxmsgsize, int64_t(100LL*1024LL*1024LL));
	m_socket->set(zmq::sockopt::multicast_hops, 1);
	m_socket->set(zmq::sockopt::rcvbuf, 10*1024*1024);
	m_socket->set(zmq::sockopt::rcvtimeo, 10*1000);
	m_socket->set(zmq::sockopt::sndbuf, 10*1024*1024);
	m_socket->set(zmq::sockopt::sndtimeo, 10*1000);
	// m_socket->set(zmq::sockopt::tos, 0);

	m_socket->connect(endpoint);	
	
	std::vector<zmq::message_t> rcv_msgs;
	for(size_t i = 0; i < 10; i++)
	{
		rcv_msgs.clear();
		try
		{
			zmq::recv_multipart(*m_socket, std::back_inserter(rcv_msgs));
		}
		catch(const zmq::error_t& e)
		{
			std::cout << "Got error: " << e.num() << ": " << e.what() << std::endl;
			continue;
		}

		if(rcv_msgs.empty())
		{
			std::cout << "Got empty msg" << std::endl;
			continue;	
		}

		//get connection property
		std::cout << "X-Opaleye-api is " << rcv_msgs[0].gets("X-Opaleye-api") << std::endl;

		std::cout << "Got message with " << rcv_msgs.size() << " parts:\n";
		for(size_t i = 0; i < rcv_msgs.size(); i++)
		{
			// std::cout << "\tPart " << i << ": " << std::string(rcv_msgs[i].data<char>(), rcv_msgs[i].size());
			std::cout << "\tPart " << i << ": " << rcv_msgs[i].to_string_view();
			std::cout << std::endl;
		}
	}

	m_socket->close();
	m_context->shutdown();
	m_context->close();

	return 0;
}
