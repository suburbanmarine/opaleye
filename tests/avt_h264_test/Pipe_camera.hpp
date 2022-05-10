#pragma once

#include <gst/gst.h>
#include <gst/app/gstappsrc.h>

#include <cassert>

class Pipe_camera
{
public:
	Pipe_camera()
	{
		appsrc = gst_element_factory_make("appsrc", NULL);
		assert(appsrc);
		g_object_set(appsrc, "is-live",      TRUE, NULL);
		g_object_set(appsrc, "do-timestamp", TRUE, NULL);
		g_object_set(appsrc, "block",        FALSE, NULL);
		g_object_set(appsrc, "min-latency",  GST_SECOND / 20L, NULL);
		// g_object_set(appsrc, "max-latency",  GST_SECOND / 2L, NULL);
		g_object_set(appsrc, "num-buffers",  10, NULL);
		g_object_set(appsrc, "max-bytes",    2464ULL*2056ULL*4ULL*10ULL, NULL);
		g_object_set(appsrc, "emit-signals", FALSE, NULL);
		g_object_set(appsrc, "stream-type",  GST_APP_STREAM_TYPE_STREAM, NULL);
		g_object_set(appsrc, "format",       GST_FORMAT_TIME, NULL);

		// all three of these work
		GstCaps* caps = gst_caps_from_string("video/x-raw, format=RGB, framerate=0/1, max-framerate=20/1, pixel-aspect-ratio=1/1, width=640, height=480, interlace-mode=progressive, colorimetry=sRGB");
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
		gst_app_src_set_caps(GST_APP_SRC(appsrc), caps);

		queue = gst_element_factory_make("queue", NULL);
		assert(queue);
	    g_object_set(queue, "leaky", 1, NULL);
	    g_object_set(queue, "max-size-buffers", 15, NULL);
	    g_object_set(queue, "max-size-bytes",   0,  NULL);
	    g_object_set(queue, "max-size-time",    0,  NULL);

		videoconvert = gst_element_factory_make("videoconvert", NULL);
		assert(videoconvert);

		tee = gst_element_factory_make("tee", NULL);
		assert(tee);
	}

	void add_to_bin(GstElement* bin)
	{
		gst_bin_add(GST_BIN(bin), appsrc);
		gst_bin_add(GST_BIN(bin), queue);
		gst_bin_add(GST_BIN(bin), videoconvert);
		gst_bin_add(GST_BIN(bin), tee);

		gst_element_link_many(appsrc, queue, videoconvert, tee, NULL);
	}

	void link_back(GstElement* next)
	{
		gst_element_link(tee, next);
	}

	GstElement* appsrc;
	GstElement* queue;
	GstElement* videoconvert;
	GstElement* tee;
};
