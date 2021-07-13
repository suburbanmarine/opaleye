#pragma once

#include "i2c_linux.hpp"

#include <cstdint>
#include <memory>

class MS5837_30BA
{
public:
	bool init(const std::shared_ptr<i2c_iface>& i2c);

	bool poll();
	
	struct CAL_DATA
	{
		uint16_t crc_factory;
		uint16_t c1; // SENS_T1
		uint16_t c2; // OFF_T1
		uint16_t c3; // TCS
		uint16_t c4; // TCO
		uint16_t c5; // Tref
		uint16_t c6; // TEMPSENS
	};

	double calc_pressure(const uint32_t d1, const uint32_t d2, const CAL_DATA& data);

	bool read_cal_data(CAL_DATA* const out_data);
	bool read_cal_data();

	bool reset();

protected:
	std::shared_ptr<i2c_iface> m_i2c;
};
