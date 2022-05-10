#pragma once

#include <gst/gst.h>

#include <cassert>

class Pipe_disk
{
public:
	Pipe_disk();

	void add_to_bin(GstElement* bin)
	{
		gst_bin_add(GST_BIN(bin), queue);
		gst_bin_add(GST_BIN(bin), splitmuxsink);

		gst_element_link_many(queue, splitmuxsink, NULL);
	}

	GstElement* queue;
	GstElement* splitmuxsink;

	gchararray handle_format_location(GstElement* splitmux, guint fragment_id)
	{
		return g_strdup_printf("/tmp/foo-%u.mkv", fragment_id);
	}
};
