#include "opaleye-util/thread_base.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

thread_base::thread_base() : m_keep_running(false)
{

}
thread_base::~thread_base()
{

}

void thread_base::launch()
{
	if( ! m_thread.joinable() )
	{
		m_keep_running = true;
		m_thread = std::thread(&thread_base::dispatch_work, this);
	}
}

void thread_base::work()
{

}

//MT safe
void thread_base::interrupt()
{
	m_keep_running.store(false);
	m_keep_running_cv.notify_all();
}
void thread_base::join()
{
	if(joinable())
	{
		m_thread.join();
	}
}
bool thread_base::joinable() const
{
	return m_thread.joinable();
}

void thread_base::wait_for_interruption()
{
	std::unique_lock<std::mutex> lock(m_mutex);
	m_keep_running_cv.wait(lock, std::bind(&thread_base::is_interrupted, this));
}

void thread_base::dispatch_work()
{
	SPDLOG_DEBUG("Thread started: {}", std::this_thread::get_id());
	try
	{
		work();
	}
	catch(const std::exception& e)
	{
		SPDLOG_DEBUG("Thread caught exception {:s}", e.what());
		throw;
	}
	catch(...)
	{
		SPDLOG_DEBUG("Thread caught exception ...");
		throw;
	}

	SPDLOG_DEBUG("Thread exiting: {}", std::this_thread::get_id());
}
