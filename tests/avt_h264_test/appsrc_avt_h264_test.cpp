// #include "pipeline/camera/GST_v4l2_api.hpp"

#include "Pipe_camera.hpp"
#include "Pipe_disk.hpp"
#include "Pipe_display.hpp"
#include "Pipe_h264.hpp"
#include "Pipe_rtp.hpp"


#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <cassert>

#include <memory>
#include <thread>
#include <functional>

class App_stuff
{
public:

	App_stuff()
	{
		loop = g_main_loop_new(NULL, TRUE);
		assert(loop);

		pipe = gst_pipeline_new(NULL);
		assert(pipe);

		cam.add_to_bin(pipe);
		disp.add_to_bin(pipe);
		h264.add_to_bin(pipe);
		rtp.add_to_bin(pipe);
		disk.add_to_bin(pipe);

		cam.link_back(disp.queue1);
		cam.link_back(h264.queue);
		
		h264.link_back(rtp.queue);
		h264.link_back(disk.queue);
	}

	GMainLoop*  loop;

	GstElement* pipe;

	Pipe_camera cam;

	Pipe_display disp;
	Pipe_h264 h264;

	Pipe_rtp  rtp;
	Pipe_disk disk;
	
	void push_data_thread()
	{

		//app src issue -
		// https://forums.developer.nvidia.com/t/error-from-element-gstpipeline-pipeline0-gstnvarguscamerasrc-timeout/170121
		// https://stackoverflow.com/questions/59954227/gstreamer-pipeline-is-hanging-randomly
		// https://www.reddit.com/r/rust/comments/ev7m5q/gstreamerrs_pipeline_is_hanging_randomly/
		// https://gitlab.freedesktop.org/gstreamer/gstreamer-rs/-/issues/235

		g_signal_emit_by_name(rtp.multiudpsink, "add", "192.168.5.54", 5000);
		g_signal_emit_by_name(rtp.multiudpsink, "add", "127.0.0.1", 5000);

	 	for(int i = 0; i < 1000; i++)
	 	{
			std::this_thread::sleep_for(std::chrono::milliseconds(50));

			// GstBuffer* buf = gst_buffer_new_and_alloc (640*480*3);
			// GstMapInfo map;
			// gst_buffer_map (buf, &map, GST_MAP_WRITE);
			// memset (map.data, i, 640*480*3);
			// gst_buffer_unmap (buf, &map);

			// gst_app_src_push_buffer(GST_APP_SRC(cam.appsrc), buf);


			if((i % 20) == 0)
			{
				gst_debug_bin_to_dot_file(GST_BIN(pipe), GST_DEBUG_GRAPH_SHOW_ALL, "test.dot");
			}
	 	}

		// gst_app_src_end_of_stream(GST_APP_SRC(cam.appsrc));
	}
};


int main(int argc, char* argv[])
{
	setenv("GST_DEBUG_DUMP_DOT_DIR", "/tmp", TRUE);

	gst_init(&argc, &argv);

	gst_debug_set_default_threshold(GST_LEVEL_INFO);

	std::shared_ptr<App_stuff> app = std::make_shared<App_stuff>();
	assert(app);

 	gst_element_set_state(app->pipe, GST_STATE_PLAYING);

 	std::thread m_thread(std::bind(&App_stuff::push_data_thread, app.get()));

	g_main_loop_run(app->loop);

	gst_element_set_state(app->pipe, GST_STATE_NULL);

	m_thread.join();

	return 0;
}