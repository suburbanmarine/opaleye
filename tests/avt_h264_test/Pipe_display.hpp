#pragma once

#include <gst/gst.h>

#include <cassert>

class Pipe_display
{
public:
	Pipe_display()
	{
		queue = gst_element_factory_make("queue", NULL);
		assert(queue);
	    g_object_set(queue, "leaky", 1, NULL);
	    g_object_set(queue, "max-size-buffers", 15, NULL);
	    g_object_set(queue, "max-size-bytes",   0,  NULL);
	    g_object_set(queue, "max-size-time",    0,  NULL);

		videoconvert = gst_element_factory_make("videoconvert", NULL);
		assert(videoconvert);

		disp = gst_element_factory_make("autovideosink", NULL);
		assert(disp);
	}

	void add_to_bin(GstElement* bin)
	{
		gst_bin_add(GST_BIN(bin), queue);
		gst_bin_add(GST_BIN(bin), videoconvert);
		gst_bin_add(GST_BIN(bin), disp);

		gst_element_link_many(queue, videoconvert, disp, NULL);
	}

	GstElement* queue;
	GstElement* videoconvert;
	GstElement* disp;

};
