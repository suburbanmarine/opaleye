#include "Unit_conv.hpp"

constexpr double Unit_conv::FEET_TO_METER;
constexpr double Unit_conv::METER_TO_FEET;

double Unit_conv::degC_to_degF(const double val)
{
	return val * 9.0 / 5.0 + 32.0;
}
double Unit_conv::degF_to_degC(const double val)
{
	return (val - 32.0) * 5.0 / 9.0;
}

double Unit_conv::meter_to_feet(const double val)
{
	return val * METER_TO_FEET;
}
double Unit_conv::feet_to_meter(const double val)
{
	return val * FEET_TO_METER;
}