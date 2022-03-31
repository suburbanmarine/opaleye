#include "cameras/Alvium_v4l2.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <thread>

void new_frame_cb(const ConstMmapFramePtr& frame)
{
	if(frame)
	{
		switch(frame->get_fmt().type)
		{
			case V4L2_BUF_TYPE_VIDEO_CAPTURE:
			{
				break;
			}
			default:
			{
				SPDLOG_ERROR("unhandled frame type");
				break;
			}
		}
	}
}

int main()
{
	Alvium_v4l2 cam;

	if( ! cam.open("/dev/video0") )
	{
		SPDLOG_ERROR("cam.open failed");
		return -1;
	}

	if( ! cam.init("cam0") )
	{
		SPDLOG_ERROR("cam.init failed");
		return -1;
	}

	if( ! cam.start_streaming() )
	{
		SPDLOG_ERROR("cam.start_streaming() failed");
		return -1;
	}

	if( ! cam.set_sw_trigger() )
	{
		SPDLOG_ERROR("cam.set_sw_trigger() failed");
		return -1;
	}

	
	for(int i = 0; i < 10; i++)
	{
		if( ! cam.send_software_trigger() )
		{
			SPDLOG_ERROR("cam.send_software_trigger() failed");
			return -1;
		}

		if( ! cam.wait_for_frame(std::chrono::milliseconds(250)) )
		{
			SPDLOG_ERROR("cam.wait_for_frame() failed");
			return -1;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	if( ! cam.stop_streaming() )
	{
		SPDLOG_ERROR("cam.stop_streaming() failed");
		return -1;
	}

	if( ! cam.close() )
	{
		SPDLOG_ERROR("cam.close() failed");
		return -1;
	}

	return 0;
}