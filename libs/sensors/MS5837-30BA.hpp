/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "i2c_linux.hpp"

#include <cstdint>
#include <memory>

class MS5837_30BA
{
public:
	bool init(const std::shared_ptr<i2c_iface>& i2c);

	bool sample(uint32_t* out_d1, uint32_t* out_d2);
	bool reset();
	
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

	struct RESULT
	{
		int32_t P1_mbar;
		int32_t TEMP1_cdegC;
		int32_t P2_mbar;
		int32_t TEMP2_degC;
	};

	static void calc_pressure(const uint32_t d1, const uint32_t d2, const CAL_DATA& data, RESULT* const out_result);

	bool read_cal_data(CAL_DATA* const out_data);

protected:
	std::shared_ptr<i2c_iface> m_i2c;
};
