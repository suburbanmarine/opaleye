#include "sensor_thread.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#include <thread>

thread_base::thread_base() : m_keep_running(true)
{

}
thread_base::~thread_base()
{

}

void thread_base::launch()
{
	m_thread = std::thread(&thread_base::work, this);
}

void thread_base::work()
{

}

void thread_base::interrupt()
{
	m_keep_running = false;
}

//MT safe
void thread_base::join()
{
	m_thread.join();
}

sensor_thread::sensor_thread()
{

}
sensor_thread::~sensor_thread()
{
	
}

bool sensor_thread::init()
{
	m_i2c = std::make_shared<i2c_linux>();
	bool ret = m_i2c->init("/dev/i2c-0");
	if(!ret)
	{
	    SPDLOG_ERROR("I2C init failed");
		return false;
	}

	ret = m_temp.reset();
	if(!ret)
	{
		SPDLOG_ERROR("temp reset failed");
		return false;
	}

	ret = m_baro.reset();
	if(!ret)
	{
		SPDLOG_ERROR("baro reset failed");
		return false;
	}


	ret = m_temp.read_cal_data(&m_temp_cal_data);
	if(!ret)
	{
		SPDLOG_ERROR("temp cal data failed");
		return false;
	}


	ret = m_baro.read_cal_data(&m_baro_cal_data);
	if(!ret)
	{
		SPDLOG_ERROR("baro cal data failed");
		return false;
	}

	return true;
}

void sensor_thread::work()
{	

	while(m_keep_running)
	{
		bool ret = m_temp.sample();
		if(!ret)
		{
			SPDLOG_WARN("temp sample failed");
		}

		ret = m_baro.sample();
		if(!ret)
		{
			SPDLOG_WARN("baro sample failed");
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5 * 1000));
	}
}