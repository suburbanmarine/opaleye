#include "Pipe_disk.hpp"

namespace
{
	gchararray dispatch_format_location(GstElement* splitmux, guint fragment_id, gpointer udata)
	{
		Pipe_disk* inst = (Pipe_disk*) udata;
		return inst->handle_format_location(splitmux, fragment_id);
	}
}

Pipe_disk::Pipe_disk()
{
	queue = gst_element_factory_make("queue", NULL);
	assert(queue);
    // g_object_set(queue, "leaky", 1, NULL);
    g_object_set(queue, "max-size-buffers", 15, NULL);
    g_object_set(queue, "max-size-bytes",   0,  NULL);
    g_object_set(queue, "max-size-time",    0,  NULL);

	splitmuxsink = gst_element_factory_make("splitmuxsink", NULL);
	assert(splitmuxsink);
	g_object_set(splitmuxsink, "max-files",      0, NULL);
	g_object_set(splitmuxsink, "max-size-bytes", 0, NULL);
	g_object_set(splitmuxsink, "max-size-time",  10*60*GST_SECOND, NULL);
// 	// m_splitmuxsink->set_property("send-keyframe-requests",  true); // max-size-bytes must be 0
	// g_object_set(splitmuxsink, "muxer", "avimux", NULL);
	// g_object_set(splitmuxsink, "muxer", "avimux", NULL);
	g_signal_connect(splitmuxsink, "format-location", G_CALLBACK(&dispatch_format_location), this);
}
