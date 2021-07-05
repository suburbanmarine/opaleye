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

	m_mux.set_top_storage_dir(top_storage_dir);
	if( ! m_mux.init("mux0") )
	{
		return false;
	}

	m_interpipe_src.add_to_bin(m_pipeline);
	m_mux.add_to_bin(m_pipeline);

	m_interpipe_src.link_back(m_mux.front());

	return true;
}

void gst_filesink_pipeline::set_listen_to(const char name[])
{
	m_interpipe_src.set_listen_to(name);
}

void gst_filesink_pipeline::send_pipeline_eos()
{
  Glib::RefPtr<Gst::EventEos> eos = Gst::EventEos::create();
  m_pipeline->send_event(eos);
}
void gst_filesink_pipeline::wait_pipeline_eos()
{
	std::unique_lock<std::mutex> lock(m_mutex_got_eos);
	m_cv_got_eos.wait(lock, [this]{return m_got_eos.load();});
}
