#include "Pipe_camera.hpp"

#include <iostream>
#include <thread>

int read_data_idle(gpointer udata)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(50));

	Pipe_camera* cam = static_cast<Pipe_camera*>(udata);

	std::cout << "read_data_idle pushing frame " << cam->m_frame_ctr << std::endl;

	GstBuffer* buf = gst_buffer_new_and_alloc (640*480*3);
	GstMapInfo map;
	gst_buffer_map (buf, &map, GST_MAP_WRITE);
	memset (map.data, cam->m_frame_ctr, 640*480*3);
	gst_buffer_unmap (buf, &map);

	GstFlowReturn ret = gst_app_src_push_buffer(GST_APP_SRC(cam->appsrc), buf);

	// GstFlowReturn ret;
	// g_signal_emit_by_name(cam->appsrc, "push-buffer", buf, &ret);
	// gst_buffer_unref (buf);

	if((cam->m_frame_ctr % 50) == 0)
	{
		// gst_debug_bin_to_dot_file(GST_BIN(pipe), GST_DEBUG_GRAPH_SHOW_ALL, "test.dot");
	}

	cam->m_frame_ctr++;

	if (ret != GST_FLOW_OK) {
		/* some error, stop sending data */
		GST_DEBUG ("some error");
		return FALSE;
	}

	return TRUE;
}

void start_data_feed(GstElement* appsrc, guint length, gpointer udata)
{
	std::cout << "start_data_feed" << std::endl;

	Pipe_camera* cam = static_cast<Pipe_camera*>(udata);

	if(cam->appsrc_idle_handle == 0)
	{
		cam->appsrc_idle_handle = g_idle_add((GSourceFunc)read_data_idle, udata);
	}
}

void stop_data_feed(GstElement* appsrc, gpointer udata)
{
	std::cout << "stop_data_feed" << std::endl;

	Pipe_camera* cam = static_cast<Pipe_camera*>(udata);

	if(cam->appsrc_idle_handle != 0)
	{
		g_source_remove(cam->appsrc_idle_handle);
		cam->appsrc_idle_handle = 0;
	}
}