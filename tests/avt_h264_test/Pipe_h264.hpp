#pragma once

#include <gst/gst.h>

#include <cassert>

class Pipe_h264
{
public:
	Pipe_h264()
	{
		queue = gst_element_factory_make("queue", NULL);
		assert(queue);
	    // g_object_set(queue, "leaky", 1, NULL);
	    g_object_set(queue, "max-size-buffers", 15, NULL);
	    g_object_set(queue, "max-size-bytes",   0,  NULL);
	    g_object_set(queue, "max-size-time",    0,  NULL);

		videoconvert = gst_element_factory_make("videoconvert", NULL);
		assert(videoconvert);

		incaps = gst_element_factory_make("capsfilter", NULL);
		assert(incaps);
		{
			GstCaps* caps = gst_caps_from_string("video/x-raw, format=I420");
			assert(caps);
			g_object_set(incaps, "caps", caps,  NULL);
		}

		h264enc = gst_element_factory_make("x264enc", NULL);
		assert(h264enc);
		
		g_object_set(h264enc, "insert-vui", TRUE,  NULL);
		// g_object_set(h264enc, "key-int-max", 120,  NULL);
		g_object_set(h264enc, "bitrate",     2048,  NULL);
		// g_object_set(h264enc, "byte-stream", TRUE,  NULL);
		g_object_set(h264enc, "tune",         4,  NULL); // zero latency
		g_object_set(h264enc, "speed-preset", 1,  NULL); // ultrafast
		

		h264parse = gst_element_factory_make("h264parse", NULL);
		assert(h264parse);

		outcaps = gst_element_factory_make("capsfilter", NULL);
		assert(outcaps);
		{
			GstCaps* caps = gst_caps_from_string("video/x-h264, stream-format=avc, profile=main");
			assert(caps);
			g_object_set(outcaps, "caps", caps,  NULL);
		}

		tee = gst_element_factory_make("tee", NULL);
		assert(tee);
	}

	void add_to_bin(GstElement* bin)
	{
		gst_bin_add(GST_BIN(bin), queue);
		gst_bin_add(GST_BIN(bin), videoconvert);
		gst_bin_add(GST_BIN(bin), incaps);
		gst_bin_add(GST_BIN(bin), h264enc);
		gst_bin_add(GST_BIN(bin), h264parse);
		gst_bin_add(GST_BIN(bin), outcaps);
		gst_bin_add(GST_BIN(bin), tee);

		gst_element_link_many(queue, videoconvert, incaps, h264enc, h264parse, outcaps, tee, NULL);
	}

	void link_back(GstElement* next)
	{
		gst_element_link(tee, next);
	}

	GstElement* queue;
	GstElement* videoconvert;
	GstElement* incaps;
	GstElement* h264enc;
	GstElement* h264parse;
	GstElement* outcaps;
	GstElement* tee;

};
