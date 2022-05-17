#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>

#include <cassert>

#include <thread>
#include <iostream>

class App_stuff
{
public:

	App_stuff()
	{
		appsrc_idle_handle = NULL;
		m_frame_ctr        = 0;

		pipe         = nullptr;
		appsrc       = nullptr;
		queue1       = nullptr;
		videoconvert = nullptr;
		queue2       = nullptr;
		avsink       = nullptr;

		loop        = nullptr;
	}

	GstElement* pipe;
	GstElement* appsrc;
	GstElement* queue1;
	GstElement* videoconvert;
	GstElement* queue2;
	GstElement* avsink;

	guint appsrc_idle_handle;
	int m_frame_ctr;

	GMainLoop* loop;
};

App_stuff app;

int read_data_idle(gpointer udata)
{
	App_stuff* app = static_cast<App_stuff*>(udata);

	int ret = TRUE;

 	if(app->m_frame_ctr < 1000)
 	{
 		std::cout << "read_data_idle pushing frame " << app->m_frame_ctr << std::endl;

		GstBuffer* buf = gst_buffer_new_and_alloc (1920*1080*3);
		GstMapInfo map;
		gst_buffer_map (buf, &map, GST_MAP_WRITE);
		memset (map.data, app->m_frame_ctr, 1920*1080*3);
		gst_buffer_unmap (buf, &map);

		// GST_BUFFER_TIMESTAMP(buf) = app->m_frame_ctr * GST_MSECOND;
		// GST_BUFFER_PTS(buf)       = app->m_frame_ctr * GST_MSECOND;
		// GST_BUFFER_DURATION(buf)  = 20 * GST_MSECOND;

		gst_app_src_push_buffer(GST_APP_SRC(app->appsrc), buf);

		app->m_frame_ctr++;
		ret = TRUE;
 	}
 	else
 	{
 		std::cout << "read_data_idle pushing eos" << std::endl;
		gst_app_src_end_of_stream(GST_APP_SRC(app->appsrc));
		ret = FALSE;
 	}

	return ret;
}
void start_data_feed(GstElement* appsrc, guint length, gpointer udata)
{
	std::cout << "start_data_feed" << std::endl;

	App_stuff* app = static_cast<App_stuff*>(udata);

	if(app->appsrc_idle_handle == 0)
	{
		app->appsrc_idle_handle = g_idle_add((GSourceFunc)read_data_idle, udata);
	}
}
void stop_data_feed(GstElement* appsrc, gpointer udata)
{
	std::cout << "stop_data_feed" << std::endl;

	App_stuff* app = static_cast<App_stuff*>(udata);

	if(app->appsrc_idle_handle != 0)
	{
		g_source_remove(app->appsrc_idle_handle);
		app->appsrc_idle_handle = 0;
	}
}

gboolean app_bus_callback(GstBus * bus, GstMessage * message, gpointer data)
{
	g_print ("Got %s message\n", GST_MESSAGE_TYPE_NAME (message));

	switch (GST_MESSAGE_TYPE(message))
	{
    	case GST_MESSAGE_ERROR:
    	{
			GError *err;
			gchar *debug;

			gst_message_parse_error (message, &err, &debug);
			g_print ("Error: %s\n", err->message);
			g_error_free (err);
			g_free (debug);

			g_main_loop_quit(app.loop);
    	  	break;
		}
    }

	return TRUE;
}

int main(int argc, char* argv[])
{
	gst_init (&argc, &argv);

	gst_debug_set_default_threshold(GST_LEVEL_INFO);

	app.loop = g_main_loop_new(NULL, TRUE);

	app.pipe   = gst_pipeline_new(NULL);
	assert(app.pipe);

	// GstBus* bus = gst_pipeline_get_bus(GST_PIPELINE(app.pipe));
	// guint bus_watch_id = gst_bus_add_watch(bus, app_bus_callback, NULL);
	// gst_object_unref(bus);

	app.appsrc = gst_element_factory_make("appsrc", NULL);
	assert(app.appsrc);
	gst_bin_add(GST_BIN(app.pipe), app.appsrc);
	g_object_set(app.appsrc, "is-live",      TRUE, NULL);
	g_object_set(app.appsrc, "do-timestamp", TRUE, NULL);
	// g_object_set(app.appsrc, "block",        FALSE, NULL);
	g_object_set(app.appsrc, "min-latency",  GST_SECOND / 20L, NULL);
	// g_object_set(app.appsrc, "num-buffers",  10, NULL); // <- this breaks it!!!!!!
	g_object_set(app.appsrc, "format",       GST_FORMAT_TIME, NULL);
	g_object_set(app.appsrc, "emit-signals", TRUE, NULL);
	g_signal_connect(app.appsrc, "need-data",    G_CALLBACK(&start_data_feed), &app);
	g_signal_connect(app.appsrc, "enough-data",    G_CALLBACK(&stop_data_feed), &app);

	app.queue1 = gst_element_factory_make("queue", NULL);
	assert(app.queue1);
	gst_bin_add(GST_BIN(app.pipe), app.queue1);
	g_object_set(app.queue1, "max-size-buffers", 15, NULL);
	g_object_set(app.queue1, "max-size-bytes",   0,  NULL);
	g_object_set(app.queue1, "max-size-time",    0,  NULL);

	// all three of these work
	GstCaps* caps = gst_caps_from_string("video/x-raw, format=RGB, framerate=0/1, max-framerate=20/1, pixel-aspect-ratio=1/1, width=1920, height=1080, interlace-mode=progressive, colorimetry=sRGB");
	// GstCaps* caps = gst_caps_from_string("video/x-raw,format=RGB,framerate=20/1,width=1920,height=1080,interlace-mode=progressive");
	// GstCaps* caps = gst_caps_new_simple("video/x-raw",
	// 	"format",         G_TYPE_STRING,     "RGB",
	// 	"framerate",      GST_TYPE_FRACTION,  20, 1,
	// 	"width",          G_TYPE_INT,         1920,
	// 	"height",         G_TYPE_INT,         1080,
	// 	"interlace-mode", G_TYPE_STRING,      "progressive",
	// 	NULL
	// 	);
	assert(caps);
	gst_app_src_set_caps(GST_APP_SRC(app.appsrc), caps);

	app.videoconvert = gst_element_factory_make("videoconvert", NULL);
	assert(app.videoconvert);
	gst_bin_add(GST_BIN(app.pipe), app.videoconvert);

	app.queue2 = gst_element_factory_make("queue", NULL);
	assert(app.queue2);
	gst_bin_add(GST_BIN(app.pipe), app.queue2);
	g_object_set(app.queue2, "max-size-buffers", 15, NULL);
	g_object_set(app.queue2, "max-size-bytes",   0,  NULL);
	g_object_set(app.queue2, "max-size-time",    0,  NULL);

	app.avsink = gst_element_factory_make("autovideosink", NULL);
	assert(app.avsink);
	gst_bin_add(GST_BIN(app.pipe), app.avsink);

	gst_element_link(app.appsrc, app.queue1);
	gst_element_link(app.queue1, app.videoconvert);
	gst_element_link(app.videoconvert, app.queue2);
	gst_element_link(app.queue2, app.avsink);

 	gst_element_set_state(app.pipe, GST_STATE_PLAYING);

	g_main_loop_run(app.loop);

	std::cout << "main loop exit" << std::endl;

	return 0;
}