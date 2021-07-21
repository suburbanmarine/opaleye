#pragma once

#include "i2c_linux.hpp"

#include <cstdint>
#include <memory>

class TSYS01
{
public:
	bool init(const std::shared_ptr<i2c_iface>& i2c);

	bool sample(uint32_t* out_sample);

	struct CAL_DATA
	{
		uint16_t k4;
		uint16_t k3;
		uint16_t k2;
		uint16_t k1;
		uint16_t k0;
	};

	static double calc_temp(const uint32_t sample, const CAL_DATA& data);

	bool read_cal_data(CAL_DATA* const out_data);
	bool read_cal_data();

	bool reset();

protected:

	std::shared_ptr<i2c_iface> m_i2c;

	uint8_t m_dev_addr;
	CAL_DATA m_cal_data;
};
