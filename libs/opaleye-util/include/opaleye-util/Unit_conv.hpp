/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

class Unit_conv
{
public:
	static double degC_to_degF(const double val);
	static double degF_to_degC(const double val);

	static double meter_to_feet(const double val);
	static double feet_to_meter(const double val);

	static constexpr double FEET_TO_METER = 0.3048; // by definition
	static constexpr double METER_TO_FEET = 1.0 / FEET_TO_METER;
};
