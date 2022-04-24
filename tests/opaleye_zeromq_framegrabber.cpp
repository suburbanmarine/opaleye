#include <zmq.hpp>
#include <zmq_addon.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/printf.h>

#include <iostream>
#include <string>
#include <memory>

int main(int argc, char* argv[])
{
	//Parse program options
	boost::program_options::variables_map vm;
	{
		//register options
		namespace bpo = boost::program_options;
		bpo::options_description desc("Options"); 
	    desc.add_options() 
			("help"      , "Print usage information and exit")
			("num_frames", bpo::value<int>()->default_value(10),                                       "Number of frames to grab")
			("endpoint",   bpo::value<std::string>()->default_value("tcp://192.168.5.7:51000"),        "Host to connect to")
			("topic",      bpo::value<std::string>()->default_value("/api/v1/cameras/cam0/live/full"), "The topic to listen to")
			("display", "Display to window")
			;

		//Parse options
	    try
	    {
	    	bpo::store(bpo::parse_command_line(argc, argv, desc), vm);

			if(vm.count("help"))
			{
				std::cout << desc << std::endl;
				return 0;
			}

	    	bpo::notify(vm);
	    }
	    catch(const bpo::error& e)
	    {
	    	std::cout << e.what() << std::endl;
		  	std::cout << desc << std::endl;
			return -1;
	    }
	}

	std::shared_ptr<zmq::context_t> m_context = std::make_shared<zmq::context_t>();

	const std::string endpoint = vm["endpoint"].as<std::string>();
	std::shared_ptr<zmq::socket_t> m_socket  = std::make_shared<zmq::socket_t>(*m_context, zmq::socket_type::sub);
    
    const std::string topic = vm["topic"].as<std::string>();
    m_socket->set(zmq::sockopt::subscribe, topic.c_str());
    m_socket->set(zmq::sockopt::connect_timeout, 5*1000);
	m_socket->set(zmq::sockopt::ipv6, 0);
	m_socket->set(zmq::sockopt::linger, 0);
	m_socket->set(zmq::sockopt::maxmsgsize, int64_t(50LL*1024LL*1024LL));
	m_socket->set(zmq::sockopt::multicast_hops, 1);
	m_socket->set(zmq::sockopt::rcvbuf, 100*1024*1024);
	m_socket->set(zmq::sockopt::rcvtimeo, 10*1000);
	m_socket->set(zmq::sockopt::sndbuf, 100*1024*1024);
	m_socket->set(zmq::sockopt::sndtimeo, 10*1000);
	m_pub_socket->set(zmq::sockopt::rcvhwm, 10);
	m_pub_socket->set(zmq::sockopt::sndhwm, 10);
	// m_socket->set(zmq::sockopt::tos, 0);

	m_socket->connect(endpoint);	
	
	int col = 2464;
	int row = 2056;
	cv::Mat disp_img(cv::Size2i(col, row), CV_8UC3);

	bool keep_going = true;
	size_t frame_ctr  = 0;
	const int num_frames = vm["num_frames"].as<int>();
	std::vector<zmq::message_t> rcv_msgs;
	while(keep_going)
	{
		if(num_frames > 0)
		{
			if(frame_ctr > num_frames)
			{
				keep_going = false;
				break;
			}
		}
		else
		{
			keep_going = true;
		}

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
		if(false)
		{
			std::cout << "X-Opaleye-api is " << rcv_msgs[0].gets("X-Opaleye-api") << std::endl;

			std::cout << "Got message with " << rcv_msgs.size() << " parts:\n";
			for(size_t i = 0; i < rcv_msgs.size(); i++)
			{
				// std::cout << "\tPart " << i << ": " << std::string(rcv_msgs[i].data<char>(), rcv_msgs[i].size());
				if(i < 2)
				{
					std::cout << "\tPart " << i << ": " << rcv_msgs[i].to_string_view();
				}
				else
				{
					std::cout << "\tPart " << i << " is " << rcv_msgs[i].to_string_view().size() << " long";	
				}
				std::cout << std::endl;
			}
		}
		else
		{
			if(rcv_msgs.size() == 3)
			{


				std::cout << "Frame: " << frame_ctr << std::endl;

				if(vm.count("display"))
				{
					cv::Mat img(cv::Size2i(col, row), CV_8UC4, (void*)rcv_msgs[2].to_string_view().data(), cv::Mat::AUTO_STEP);

					cv::cvtColor(img, disp_img, cv::COLOR_BGRA2BGR);

					cv::imshow("Frame", disp_img);
					cv::waitKey(1);
				}
			}
			else
			{
				std::cout << "Got message with " << rcv_msgs.size() << " parts:";
			}
		}

		frame_ctr++;
	}

	m_socket->close();
	m_context->shutdown();
	m_context->close();

	return 0;
}
