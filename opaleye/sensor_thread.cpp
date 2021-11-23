/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "sensor_thread.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

sensor_thread::sensor_thread()
{
	m_temp_degC = 0.0;
	m_baro_data = {};
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

	while( ! is_interrupted() )
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

			SPDLOG_DEBUG("Temperature is {:f} degC", ext_temp);
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

		wait_for_interruption(std::chrono::milliseconds(2 * 1000));
	}
}