/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "MS5837-30BA.hpp"
#include "TSYS01.hpp"

#include "thread_base.hpp"

#include <mutex>

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