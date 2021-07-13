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
	m_thread = std::thread(&thread_base::dispatch_work, this);
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

void thread_base::dispatch_work()
{
	SPDLOG_DEBUG("Thread started");
	try
	{
		work();
	}
	catch(const std::exception& e)
	{
		SPDLOG_DEBUG("Thread caught exception {:s}", e.what());	
	}
	catch(...)
	{
		SPDLOG_DEBUG("Thread caught exception ...");	
	}

	SPDLOG_DEBUG("Thread exiting");
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

	ret = m_temp.init(m_i2c);
	if(!ret)
	{
		SPDLOG_ERROR("temp init failed");
		return false;
	}

	ret = m_baro.init(m_i2c);
	if(!ret)
	{
		SPDLOG_ERROR("baro init failed");
		return false;
	}

	ret = m_temp.reset();
	if(!ret)
	{
		SPDLOG_ERROR("temp reset failed");
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(3));

	ret = m_baro.reset();
	if(!ret)
	{
		SPDLOG_ERROR("baro reset failed");
		return false;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(3));

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
	double ext_temp = 0.0;
	MS5837_30BA::RESULT baro_result;

	while(m_keep_running)
	{
		uint32_t temp_sample;
		bool ret = m_temp.sample(&temp_sample);
		if(!ret)
		{
			SPDLOG_WARN("temp sample failed");
		}
		else
		{
			ext_temp = TSYS01::calc_temp(temp_sample, m_temp_cal_data);

			SPDLOG_DEBUG("Temperature is {:f} degC", temp);
		}

		uint32_t d1, d2;
		ret = m_baro.sample(&d1, &d2);
		if(!ret)
		{
			SPDLOG_WARN("baro sample failed");
		}
		else
		{
			MS5837_30BA::calc_pressure(d1, d2, m_baro_cal_data, &baro_result);

			SPDLOG_DEBUG("Pressure is {:d} mbar", baro_result.P2_mbar);
		}

		{
			std::unique_lock<std::mutex> lock;
			m_temp_degC = ext_temp;
			m_baro_data = baro_result;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5 * 1000));
	}
}