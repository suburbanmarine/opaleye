#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <cassert>

#include <thread>

class App_stuff
{
public:
	GstElement* pipe;
	GstElement* appsrc;
	GstElement* avsink;

	GMainLoop* loop;
};

App_stuff app;

void push_data_thread()
{
 	for(int i = 0; i < 1000; i++)
 	{
		GstBuffer *buf;
		GstMapInfo map;

		buf = gst_buffer_new_and_alloc (1920*1080*3);
		gst_buffer_map (buf, &map, GST_MAP_WRITE);
		memset (map.data, i, 100);
		gst_buffer_unmap (buf, &map);

		gst_app_src_push_buffer (GST_APP_SRC(app.appsrc), buf);

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
 	}
}

int main(int argc, char* argv[])
{
	gst_init (&argc, &argv);

	gst_debug_set_default_threshold(GST_LEVEL_INFO);

	app.loop = g_main_loop_new(NULL, TRUE);

	app.pipe   = gst_pipeline_new (NULL);
	assert(app.pipe);

	app.appsrc = gst_element_factory_make ("appsrc", NULL);
	assert(app.appsrc);
	gst_bin_add(GST_BIN(app.pipe), app.appsrc);

	GstCaps* caps = gst_caps_from_string("video/x-raw, format=RGB, framerate=0/1, max-framerate=20/1, pixel-aspect-ratio=1/1, width=1920, height=1080, interlace-mode=progressive, colorimetry=sRGB");
	assert(caps);
	gst_app_src_set_caps(GST_APP_SRC(app.appsrc), caps);

	app.avsink = gst_element_factory_make ("autovideosink", NULL);
	assert(app.avsink);
	gst_bin_add(GST_BIN(app.pipe), app.avsink);

	gst_element_link(app.appsrc, app.avsink);

 	gst_element_set_state(app.pipe, GST_STATE_PLAYING);

 	std::thread m_thread(push_data_thread);

	g_main_loop_run(app.loop);

	return 0;
}