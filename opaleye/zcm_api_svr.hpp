#pragma once

#include <opaleye-util/thread_base.hpp>

#include <zcm/zcm.h>

#include <memory>
#include <string>

class zcm_run_thread : public thread_base
{
public:
	zcm_run_thread(const std::shared_ptr<zcm_t>& ctx);
	~zcm_run_thread() override;

	void work();

protected:
	std::shared_ptr<zcm_t> m_ctx;
};

class zcm_api_svr
{
public:
	zcm_api_svr();
	~zcm_api_svr();

	bool init(const std::string& ep);

	bool stop();

	bool good()
	{
		if(m_context)
		{
			//return m_context->good();
		}

		return false;
	}

protected:

	std::shared_ptr<zcm_run_thread> m_zcm_run_thread;

	std::shared_ptr<zcm_t> m_context;
	std::string m_ep;
};
