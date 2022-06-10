#include "zeromq_api_svr.hpp"

#include "zmq.hpp"

#include <chrono>
#include <iostream>

int main()
{

	zeromq_api_svr api;

	std::list<std::string> zmq_ep;
	zmq_ep.push_back("tcp://127.0.0.1:50000");

	api.init(zmq_ep);

	for(;;)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
