#pragma once

#include "MS5837-30BA.hpp"
#include "TSYS01.hpp"

#include <atomic>
#include <thread>

class thread_base
{
public:
	thread_base();
	virtual ~thread_base();

	void launch();

	virtual void work();

	void interrupt();

	//MT safe
	void join();
protected:

	void dispatch_work();

	std::atomic<bool> m_keep_running;
	std::thread m_thread;
};

class sensor_thread : public thread_base
{
public:

	sensor_thread();
	~sensor_thread() override;

	bool init();

	void work() override;

protected:
	std::shared_ptr<i2c_linux> m_i2c;

	TSYS01 m_temp;
	TSYS01::CAL_DATA m_temp_cal_data;

	MS5837_30BA m_baro;
	MS5837_30BA::CAL_DATA m_baro_cal_data;

};