#include <zcm/zcm-cpp.hpp>

#include "zcm_api_svr.hpp"

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

std::mutex done_flag_mutex;
std::condition_variable done_flag;

std::atomic<int> cb_count;

void hb_handler(const zcm::ReceiveBuffer* rbuf,
	              const std::string& channel,
                  void* usr
              )
{
	std::cout << "Got message" << std::endl;
	std::cout << "\tChannel: "<< channel << std::endl;
	std::cout << "\tbody: " << std::string((const char*)rbuf->data, rbuf->data_size) << std::endl;

	cb_count++;
	done_flag.notify_all();
}

void image_handler(const zcm::ReceiveBuffer* rbuf,
	              const std::string& channel,
                  void* usr
              )
{
	std::cout << "Got message" << std::endl;
	std::cout << "\tChannel: "<< channel << std::endl;
	std::cout << "\tbody size: " << rbuf->data_size << std::endl;

	cb_count++;
	done_flag.notify_all();
}

bool handler_is_done()
{
	return cb_count.load() >= 10;
}

int main()
{
	// const std::string ep = "ipc";
	const std::string ep = "udpm://239.1.1.1:52000?ttl=1";
	
	std::shared_ptr<zcm::ZCM> m_zcm = std::make_shared<zcm::ZCM>(ep);
	if( ! m_zcm )
	{
		std::cout << "zcm_api_svr::init could not create m_zcm" << std::endl;
		return false;
	}

	std::shared_ptr<zcm_run_thread> m_zcm_run_thread = std::make_shared<zcm_run_thread>(m_zcm);
	m_zcm_run_thread->launch();

	//m_zcm->subscribe("/api/v1/heartbeat", &hb_handler, nullptr);
	m_zcm->subscribe("/api/v1/cam/cam0/live/full", &image_handler, nullptr);
	m_zcm->subscribe("/api/v1/cam/cam1/live/full", &image_handler, nullptr);
	m_zcm->subscribe("/api/v1/cam/cam2/live/full", &image_handler, nullptr);

	{
		std::unique_lock lock(done_flag_mutex);
		done_flag.wait(lock, &handler_is_done);
	}

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

	return 0;
}
