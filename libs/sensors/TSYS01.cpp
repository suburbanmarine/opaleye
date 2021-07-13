#include "TSYS01.hpp"

double TSYS01::calc_temp(const uint32_t sample, const CAL_DATA& data)
{
	const uint32_t ADC24 = sample;
	const uint32_t ADC16 = ADC24 / uint32_t(256U);

	const double ADC16_1 = double(ADC16);
	const double ADC16_2 = ADC16_1 * double(ADC16);
	const double ADC16_3 = ADC16_2 * double(ADC16);
	const double ADC16_4 = ADC16_3 * double(ADC16);

	double temp = 0.0;
	temp += -2.0 * 1e-21 * ADC16_4;
	temp +=  4.0 * 1e-26 * ADC16_3;
	temp += -2.0 * 1e-11 * ADC16_2;
	temp +=  1.0 * 1e-6  * ADC16_1;
	temp += -1.5 * 1e-2;

	return temp;
}