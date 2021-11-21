/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "MS5837-30BA.hpp"
#include "TSYS01.hpp"

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

class thread_base
{
public:
	thread_base();
	virtual ~thread_base();

	void launch();

	virtual void work();

	//MT safe
	void interrupt();
	void join();
	bool joinable() const;

	bool is_interrupted() const
	{
		return m_keep_running.load();
	}

	void wait_for_interruption()
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_keep_running_cv.wait(lock, std::bind(&thread_base::is_interrupted, this));
	}

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

class sensor_thread : public thread_base
{
public:

	sensor_thread();
	~sensor_thread() override;

	bool init();

	void work() override;

	void get_temp_data(double* out_data)
	{
		std::unique_lock<std::mutex> lock;
		*out_data = m_temp_degC;
	}

	void get_baro_data(MS5837_30BA::RESULT* out_data)
	{
		std::unique_lock<std::mutex> lock;
		*out_data = m_baro_data;
	}

protected:
	std::shared_ptr<i2c_linux> m_i2c;

	TSYS01 m_temp;
	TSYS01::CAL_DATA m_temp_cal_data;

	MS5837_30BA m_baro;
	MS5837_30BA::CAL_DATA m_baro_cal_data;

	std::mutex m_data_mutex;
	double m_temp_degC;
	MS5837_30BA::RESULT m_baro_data;
};