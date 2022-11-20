#pragma once

#include <opaleye-util/thread_base.hpp>

#include <zcm/zcm-cpp.hpp>

#include <memory>
#include <string>

class zcm_run_thread : public thread_base
{
public:
	zcm_run_thread(const std::shared_ptr<zcm::ZCM>& zcm);
	~zcm_run_thread() override;

	void work();

protected:
	std::shared_ptr<zcm::ZCM> m_zcm;
};

class zcm_hb_thread : public thread_base
{
public:
	zcm_hb_thread(const std::shared_ptr<zcm::ZCM>& zcm);
	~zcm_hb_thread() override;

	void work();

protected:
	std::shared_ptr<zcm::ZCM> m_zcm;
};

class zcm_api_svr
{
public:
	zcm_api_svr();
	~zcm_api_svr();

	bool init(const std::string& ep);

	bool stop();

	bool good() const
	{
		if(m_zcm)
		{
			return m_zcm->good();
		}

		return false;
	}

	const std::shared_ptr<zcm::ZCM>& get_zcm() const
	{
		return m_zcm;
	}

	std::shared_ptr<zcm::ZCM>& get_zcm()
	{
		return m_zcm;
	}

	template <typename T>
	int publish(const std::string& topic, const T& val)
	{
		return m_zcm->publish(topic, val);
	}

protected:

	std::shared_ptr<zcm_run_thread> m_zcm_run_thread;
	std::shared_ptr<zcm_hb_thread> m_zcm_hb_thread;

	std::shared_ptr<zcm::ZCM> m_zcm;
	std::string m_ep;
};
