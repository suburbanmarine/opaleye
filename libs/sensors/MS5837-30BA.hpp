#pragma once

#include "i2c_linux.hpp"

#include <cstdint>
#include <memory>

class TSYS01
{
public:
	bool init(const std::shared_ptr<i2c_iface>& i2c);

	bool poll();
	
	struct CAL_DATA
	{
		uint16_t crc_factory;
		uint16_t c1;
		uint16_t c2;
		uint16_t c3;
		uint16_t c4;
		uint16_t c5;
		uint16_t c6;
	};

	static double calc_pressure(const int32_t sample, const CAL_DATA& data);

protected:
	std::shared_ptr<i2c_iface> m_i2c;
};
