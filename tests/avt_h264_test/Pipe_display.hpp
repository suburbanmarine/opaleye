#pragma once

#include <gst/gst.h>

#include <cassert>

class Pipe_display
{
public:
	Pipe_display()
	{
		queue1 = gst_element_factory_make("queue", NULL);
		assert(queue1);
	    // g_object_set(queue1, "leaky", 1, NULL);
	    g_object_set(queue1, "max-size-buffers", 15, NULL);
	    g_object_set(queue1, "max-size-bytes",   0,  NULL);
	    g_object_set(queue1, "max-size-time",    0,  NULL);

		videoconvert = gst_element_factory_make("videoconvert", NULL);
		assert(videoconvert);

		queue2 = gst_element_factory_make("queue", NULL);
		assert(queue2);
	    // g_object_set(queue2, "leaky", 1, NULL);
	    g_object_set(queue2, "max-size-buffers", 15, NULL);
	    g_object_set(queue2, "max-size-bytes",   0,  NULL);
	    g_object_set(queue2, "max-size-time",    0,  NULL);

		disp = gst_element_factory_make("autovideosink", NULL);
		// g_object_set(disp, "sync",    FALSE,  NULL);
		assert(disp);
	}

	void add_to_bin(GstElement* bin)
	{
		gst_bin_add(GST_BIN(bin), queue1);
		gst_bin_add(GST_BIN(bin), videoconvert);
		gst_bin_add(GST_BIN(bin), queue2);
		gst_bin_add(GST_BIN(bin), disp);

		gst_element_link_many(queue1, videoconvert, queue2, disp, NULL);
	}

	GstElement* queue1;
	GstElement* videoconvert;
	GstElement* queue2;
	GstElement* disp;

};
