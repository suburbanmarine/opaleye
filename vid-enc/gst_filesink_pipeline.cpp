#include "gst_filesink_pipeline.hpp"

#include <spdlog/spdlog.h>

gst_filesink_pipeline::gst_filesink_pipeline()
{

}

gst_filesink_pipeline::~gst_filesink_pipeline()
{

}

bool gst_filesink_pipeline::init()
{
	if ( ! GST_app_base::init() )
	{
		SPDLOG_ERROR("Could not init GST_app_base");
		return false;
	}

	if( ! m_interpipe_src.init("h264_ipsrc_0") )
	{
		return false;
	}

	if( ! m_mkv.init("mkv0") )
	{
		return false;
	}

	m_interpipe_src.add_to_bin(m_pipeline);
	m_mkv.add_to_bin(m_pipeline);

	m_interpipe_src.link_back(m_mkv.front());

	return true;
}

void gst_filesink_pipeline::set_listen_to(const char name[])
{
	m_interpipe_src.set_listen_to(name);
}