#include "idl/zcm/heartbeat_t.hpp"
#include "idl/zcm/image_buffer_t.hpp"

#include <zcm/zcm-cpp.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/fmt/bundled/printf.h>

#include <atomic>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

class Zcm_image_buffer_t_handler
{
public:

	Zcm_image_buffer_t_handler(boost::program_options::variables_map* vm) : m_frame_ctr(0)
	{
		m_vm = vm;
	}
	~Zcm_image_buffer_t_handler()
	{

	}

	size_t frame_ctr() const
	{
		return m_frame_ctr.load();
	}

	void handle_msg(const zcm::ReceiveBuffer* rbuf, const std::string& chan, const image_buffer_t *msg)
	{
		std::cout << "Frame: "      << m_frame_ctr     << std::endl;
		std::cout << "\tMetadata: " << msg->metadata << std::endl;

		if(m_vm->count("display"))
		{
			std::vector<uint8_t> mutable_data(msg->frame.begin(), msg->frame.end());

			int col     = 0;
			int row     = 0;
			size_t step = 0;
			cv::Mat in_frame(cv::Size2i(col, row), CV_16UC1, mutable_data.data(), step);
			cv::demosaicing(in_frame, m_disp_img, cv::COLOR_BayerBG2BGR, 3);

			cv::imshow("Frame", m_disp_img);
			cv::waitKey(1);
		}

		m_frame_ctr++;
	}
protected:
	boost::program_options::variables_map* m_vm;
	std::atomic<size_t> m_frame_ctr;
	cv::Mat m_disp_img;
};

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
			("num_frames", bpo::value<size_t>()->default_value(10),                                       "Number of frames to grab")
			("endpoint",   bpo::value<std::string>()->default_value("ipc"),                            "Host to connect to")
			("topic",      bpo::value<std::string>()->default_value("/api/v1/cam/cam0/live/full"), "The topic to listen to")
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

	const std::string endpoint = vm["endpoint"].as<std::string>();
	zcm::ZCM zcm(endpoint);
    if( ! zcm.good() )
    {
        return -1;
    }
    
    const std::string topic = vm["topic"].as<std::string>();
    Zcm_image_buffer_t_handler ib_handler(&vm);
	zcm.subscribe(topic, &Zcm_image_buffer_t_handler::handle_msg, &ib_handler);

	zcm.start();

	const size_t num_frames = vm["num_frames"].as<size_t>();
	bool keep_going = true;
	while(keep_going)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));

		size_t frame_ctr = ib_handler.frame_ctr();
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
	}

	zcm.stop();

	return 0;
}
