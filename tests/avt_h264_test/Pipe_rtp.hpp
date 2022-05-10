#pragma once

#include <gst/gst.h>

#include <cassert>

class Pipe_rtp
{
public:
	Pipe_rtp()
	{
		queue = gst_element_factory_make("queue", NULL);
		assert(queue);
	    g_object_set(queue, "leaky", 1, NULL);
	    g_object_set(queue, "max-size-buffers", 15, NULL);
	    g_object_set(queue, "max-size-bytes",   0,  NULL);
	    g_object_set(queue, "max-size-time",    0,  NULL);

		h264pay      = gst_element_factory_make("rtph264pay", NULL);
		assert(h264pay);
    	g_object_set(h264pay, "config-interval", -1,     NULL);
    	g_object_set(h264pay, "name",            "pay0", NULL);
    	g_object_set(h264pay, "pt",              96,     NULL);
    	g_object_set(h264pay, "aggregate-mode",  1,      NULL);

		rtpbin       = gst_element_factory_make("queue", NULL);
		assert(rtpbin);
		g_object_set(rtpbin, "do-retransmission",  1,      NULL);

		multiudpsink = gst_element_factory_make("multiudpsink", NULL);
		assert(multiudpsink);

	}

	void add_to_bin(GstElement* bin)
	{
		gst_bin_add(GST_BIN(bin), queue);
		gst_bin_add(GST_BIN(bin), h264pay);
		gst_bin_add(GST_BIN(bin), rtpbin);
		gst_bin_add(GST_BIN(bin), multiudpsink);

		gst_element_link_many(queue, h264pay, rtpbin, multiudpsink, NULL);
	}

	GstElement* queue;
	GstElement* h264pay;
	GstElement* rtpbin;
	GstElement* multiudpsink;
};
