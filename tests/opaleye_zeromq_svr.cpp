#include "zeromq_api_svr.hpp"

#include "zmq.hpp"

#include <chrono>
#include <iostream>

int main()
{

	zeromq_api_svr api;
	api.init();

	for(;;)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}
