/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "MS5837-30BA.hpp"

#include <spdlog/spdlog.h>

#include <arpa/inet.h>

#include <thread>

bool MS5837_30BA::init(const std::shared_ptr<i2c_iface>& i2c)
{
	m_i2c = i2c;

	return true;
}
bool MS5837_30BA::sample(uint32_t* out_d1, uint32_t* out_d2)
{
	// OSR=4096 D1 (pressure)
	uint8_t d1_cmd = 0x48;
	bool ret = m_i2c->write(0x76, &d1_cmd, 1);
	if(!ret)
	{
		return false;
	}

	//wait for conversion
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::array<uint8_t, 3> d1;
	ret = m_i2c->read_cmd(0x76, 0x00, d1.data(), d1.size());
	if(!ret)
	{
		return false;
	}

	// OSR=4096 D2 (temp)
	uint8_t d2_cmd = 0x58;
	ret = m_i2c->write(0x76, &d2_cmd, 1);
	if(!ret)
	{
		return false;
	}

	//wait for conversion
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::array<uint8_t, 3> d2;
	ret = m_i2c->read_cmd(0x76, 0x00, d2.data(), d2.size());
	if(!ret)
	{
		return false;
	}

	uint32_t d1_sample = uint32_t(d1[0]) << 16 | uint32_t(d1[1]) << 8 | uint32_t(d1[2]);
	SPDLOG_DEBUG("d1_sample sample: {:d}", d1_sample);	

	uint32_t d2_sample = uint32_t(d2[0]) << 16 | uint32_t(d2[1]) << 8 | uint32_t(d2[2]);
	SPDLOG_DEBUG("d2_sample sample: {:d}", d2_sample);	

	*out_d1 = d1_sample;
	*out_d2 = d2_sample;

	return true;
}
bool MS5837_30BA::reset()
{
	std::array<uint8_t, 1> buf = {0x1E};
	bool ret = m_i2c->write(0x76, buf.data(), buf.size());
	if(!ret)
	{
		return false;
	}

	return true;
}

bool MS5837_30BA::read_cal_data(CAL_DATA* const out_data)
{
	bool ret = m_i2c->read_cmd(0x76, 0xA0, reinterpret_cast<uint8_t*>(&(out_data->crc_factory)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(0x76, 0xA2, reinterpret_cast<uint8_t*>(&(out_data->c1)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(0x76, 0xA4, reinterpret_cast<uint8_t*>(&(out_data->c2)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(0x76, 0xA6, reinterpret_cast<uint8_t*>(&(out_data->c3)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(0x76, 0xA8, reinterpret_cast<uint8_t*>(&(out_data->c4)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(0x76, 0xAA, reinterpret_cast<uint8_t*>(&(out_data->c5)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(0x76, 0xAC, reinterpret_cast<uint8_t*>(&(out_data->c6)), 2);
	if(!ret)
	{
		return false;
	}

	out_data->c1 = ntohs(out_data->c1);
	out_data->c2 = ntohs(out_data->c2);
	out_data->c3 = ntohs(out_data->c3);
	out_data->c4 = ntohs(out_data->c4);
	out_data->c5 = ntohs(out_data->c5);
	out_data->c6 = ntohs(out_data->c6);

	return true;
}

void MS5837_30BA::calc_pressure(const uint32_t d1, const uint32_t d2, const CAL_DATA& data, RESULT* const out_result)
{
	//dT = D2 - Tref = D2 - C5 * 2^8
	//TEMP = 20 + dT*TEMPSENS = 2000+dT*C6/2^23

	//OFF = OFF_T1 + TCO * dT = C2 * 2^16 + (C4 * dT / 2^7)
	//SENS = SENS_T1 + TCS*dT = C1 * 2^15 + (C3 * dT) / 2^8
	// P = D1 - SENS - OFF = D1 * (SENS / 2^21 - OFF) / 2^13

	const int32_t dT   = int32_t(d2) - int32_t(data.c5) * int32_t(1U << 8);
	const int32_t TEMP = 2000LL + int64_t(dT) * int64_t(data.c6) / (1ULL << 23);

	const int64_t OFF  = int64_t(data.c2) * (1LL << 16) + (int64_t(data.c4) * int64_t(dT)) / (1LL << 7);
	const int64_t SENS = int64_t(data.c1) * (1LL << 15) + (int64_t(data.c3) * int64_t(dT)) / (1LL << 8);

	const int32_t P1 = (int64_t(d1) * SENS / (1LL << 21) - OFF) / (1LL << 13);


	int64_t OFFi  = 0;
	int64_t SENSi = 0;
	int64_t Ti    = 0;
	if((TEMP / 100L) < 20L)
	{
		Ti = 3LL * dT*dT / (1LL<<33);
	
		OFFi  = 3LL * (TEMP - 2000LL) * (TEMP - 2000LL) / (1LL<<1);
		SENSi = 5LL * (TEMP - 2000LL) * (TEMP - 2000LL) / (1LL<<3);

		if((TEMP / 100L) < -15L)
		{
			OFFi  = OFFi  + 7LL * (TEMP + 1500LL) * (TEMP + 1500LL);
			SENSi = SENSi + 4LL * (TEMP + 1500LL) * (TEMP + 1500LL);
		}
	}
	else
	{
		Ti = 2LL * int64_t(dT)*int64_t(dT) / (1LL<<37);

		OFFi  = 1LL * (int64_t(TEMP) - 2000LL) * (int64_t(TEMP) - 2000LL) / (1LL<<4);
		SENSi = 0;
	}

	const int64_t OFF2  = OFF - OFFi;
	const int64_t SENS2 = SENS - SENSi;
	const int32_t TEMP2 = (int64_t(TEMP) - int64_t(Ti)) / (100LL);
	const int32_t P2    = (((int64_t(d1) * SENS2) / (1LL << 21) - OFF2) / (1LL<<13) / 10LL);

	SPDLOG_DEBUG("P1: {:d}, TEMP: {:d}, P2: {:d}, TEMP2: {:d}", P1, TEMP, P2, TEMP2);	

	out_result->P1_mbar     = P1;
	out_result->TEMP1_cdegC = TEMP;
	out_result->P2_mbar     = P2;
	out_result->TEMP2_degC  = TEMP2;
}