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
		uint16_t k4;
		uint16_t k3;
		uint16_t k2;
		uint16_t k1;
		uint16_t k0;
	};

	static double calc_temp(const uint32_t sample, const CAL_DATA& data);

protected:
	std::shared_ptr<i2c_iface> m_i2c;
};
