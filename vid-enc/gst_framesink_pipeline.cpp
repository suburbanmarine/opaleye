#include "gst_framesink_pipeline.hpp"

#include <spdlog/spdlog.h>

gst_framesink_pipeline::gst_framesink_pipeline()
{

}

gst_framesink_pipeline::~gst_framesink_pipeline()
{

}

bool gst_framesink_pipeline::init()
{
	if ( ! GST_app_base::init() )
	{
		SPDLOG_ERROR("Could not init GST_app_base");
		return false;
	}

	if( ! m_interpipe_src.init("frame_ipsrc_0") )
	{
		return false;
	}

	if( ! m_frame.init("frame0") )
	{
		return false;
	}

	m_interpipe_src.add_to_bin(m_pipeline);
	m_frame.add_to_bin(m_pipeline);

	m_interpipe_src.link_back(m_frame.front());

	return true;
}

void gst_framesink_pipeline::set_listen_to(const char name[])
{
	m_interpipe_src.set_listen_to(name);
}

void gst_framesink_pipeline::send_pipeline_eos()
{
  Glib::RefPtr<Gst::EventEos> eos = Gst::EventEos::create();
  m_pipeline->send_event(eos);
}
void gst_framesink_pipeline::wait_pipeline_eos()
{
	std::unique_lock<std::mutex> lock(m_mutex_got_eos);
	m_cv_got_eos.wait(lock, [this]{return m_got_eos.load();});
}
