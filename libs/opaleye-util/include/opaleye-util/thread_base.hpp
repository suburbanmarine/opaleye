#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <thread>

class thread_base
{
public:
	thread_base();
	virtual ~thread_base();

	//internal
	virtual void work();

	//not MT safe
	void launch();
	void join();
	bool joinable() const;
	
	//MT safe
	void interrupt();

	bool is_interrupted() const
	{
		return ! m_keep_running.load();
	}

	void wait_for_interruption();

	template <typename Rep, typename Period >
	bool wait_for_interruption(const std::chrono::duration<Rep, Period>& dt)
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_keep_running_cv.wait_for(lock, dt, std::bind(&thread_base::is_interrupted, this));
	}

protected:

	void dispatch_work();

	std::atomic<bool> m_keep_running;
	std::condition_variable m_keep_running_cv;
	std::mutex m_mutex;

	std::thread m_thread;
};
