#include "MS5837-30BA.hpp"

double MS5837_30BA::calc_pressure(const uint32_t d1, const uint32_t d2, const CAL_DATA& data)
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

	const int32_t P_1 = (int64_t(d1) * SENS / (1LL << 21) - OFF) / (1LL << 13);


	int64_t OFFi  = 0;
	int64_t SENSi = 0;
	int64_t Ti    = 0;
	if((TEMP / 100L) < 20L)
	{
		Ti = 2LL * dT*dT / (1LL<<37);

		OFFi  = 1LL * (TEMP - 2000LL) * (TEMP - 2000LL) / (1LL<<4);
		SENSi = 0;
	}
	else
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

	const int64_t OFF2  = OFF - OFFi;
	const int64_t SENS2 = SENS - SENSi;
	const int32_t TEMP2 = (int64_t(TEMP) - int64_t(Ti)) / (100LL);
	const int32_t P2    = (((int64_t(d1) * SENS2) / (1LL << 21) - OFF2) / (1LL<<13) / 10LL);

	return double(P2) / 1000.0;
}