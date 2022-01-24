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
	m_socket->connect(endpoint);
    m_socket->set(zmq::sockopt::subscribe, "/topic/foo");

	std::vector<zmq::message_t> rcv_msgs;
	for(size_t i = 0; i < 10; i++)
	{
		rcv_msgs.clear();
		zmq::recv_multipart(*m_socket, std::back_inserter(rcv_msgs));

		std::cout << "Got message with " << rcv_msgs.size() << " parts:\n";
		for(size_t i = 0; i < rcv_msgs.size(); i++)
		{
			// std::cout << "\tPart " << i << ": " << std::string(rcv_msgs[i].data<char>(), rcv_msgs[i].size());
			std::cout << "\tPart " << i << ": " << rcv_msgs[i].to_string();
			std::cout << std::endl;
		}
	}

	return 0;
}
