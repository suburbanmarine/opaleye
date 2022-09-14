#include "zcm_api_svr.hpp"

#include <spdlog/spdlog.h>


zcm_run_thread::zcm_run_thread(const std::shared_ptr<zcm::ZCM>& zcm)
{
	m_zcm = zcm;
}

zcm_run_thread::~zcm_run_thread()
{
	
}

void zcm_run_thread::work()
{
	SPDLOG_DEBUG("zcm_run_thread starting");

	while(!is_interrupted())
	{
		 m_zcm->run();
	}

	SPDLOG_DEBUG("zcm_run_thread stopping");
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
	if(m_zcm)
	{
		SPDLOG_INFO("zcm_api_svr::init m_zcm already created");

		return false;
	}

	m_ep = ep;
	m_zcm = std::make_shared<zcm::ZCM>(m_ep);
	if( ! m_zcm )
	{
		SPDLOG_ERROR("zcm_api_svr::init could not create m_zcm");
		return false;
	}

	m_zcm_run_thread = std::make_shared<zcm_run_thread>(m_zcm);
	m_zcm_run_thread->launch();

	return true;
}

bool zcm_api_svr::stop()
{
	if(m_zcm_run_thread)
	{
		m_zcm_run_thread->interrupt();
	}
	if(m_zcm)
	{
		m_zcm->stop();
	}
	if(m_zcm_run_thread)
	{
		m_zcm_run_thread->join();
		m_zcm_run_thread.reset();
	}
	if(m_zcm)
	{
		m_zcm.reset();
	}

	return true;
}
