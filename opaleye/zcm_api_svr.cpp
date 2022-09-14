#include "zcm_api_svr.hpp"

#include <spdlog/spdlog.h>


zcm_run_thread::zcm_run_thread(const std::shared_ptr<zcm_t>& ctx)
{
	m_ctx = ctx;
}

zcm_run_thread::~zcm_run_thread()
{
	
}

void zcm_run_thread::work()
{
	while(!is_interrupted())
	{
		 zcm_run(m_ctx.get());
	}
}

zcm_api_svr::zcm_api_svr()
{

}
zcm_api_svr::~zcm_api_svr()
{
	stop();
}

bool zcm_api_svr::init(const std::string& ep)
{
	if(m_context)
	{
		return false;
	}

	m_ep = ep;
	m_context = std::shared_ptr<zcm_t>(zcm_create(m_ep.c_str()), zcm_destroy);
	if( ! m_context )
	{
		SPDLOG_ERROR("Could not allocate context"); 
		return false;
	}

	m_zcm_run_thread = std::make_shared<zcm_run_thread>(m_context);
	m_zcm_run_thread->launch();

	return true;
}

bool zcm_api_svr::stop()
{
	if(m_context)
	{
		zcm_stop(m_context.get())
	}
	if(m_zcm_run_thread)
	{
		m_zcm_run_thread->interrupt();
		m_zcm_run_thread->join();
		m_zcm_run_thread.reset();
	}
	if(m_context)
	{
		m_context.reset();
	}

	return true;
}
