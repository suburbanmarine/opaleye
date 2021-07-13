#include "TSYS01.hpp"

#include <spdlog/spdlog.h>

#include <thread>

bool TSYS01::init(const std::shared_ptr<i2c_iface>& i2c)
{
	m_i2c = i2c;

	return true;
}

bool TSYS01::sample()
{
	std::array<uint8_t, 1> buf = {0x48};
	bool ret = m_i2c->write(m_dev_addr, buf.data(), buf.size());
	if(!ret)
	{
		return false;
	}

	//wait for conversion
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::array<uint8_t, 3> rx_buf;
	ret = m_i2c->read_cmd(m_dev_addr, 0x00, rx_buf.data(), rx_buf.size());
	if(!ret)
	{
		return false;
	}

	uint32_t sample = uint32_t(rx_buf[0]) << 16 | uint32_t(rx_buf[1]) << 8 | uint32_t(rx_buf[2]);
	SPDLOG_DEBUG("Temp sample: {:08d}", sample);	

	return true;
}

bool TSYS01::reset()
{
	std::array<uint8_t, 1> buf = {0x1E};
	bool ret = m_i2c->write(m_dev_addr, buf.data(), buf.size());
	if(!ret)
	{
		return false;
	}

	return true;
}

bool TSYS01::read_cal_data(CAL_DATA* const out_data)
{
	bool ret = m_i2c->read_cmd(m_dev_addr, 0xA2, reinterpret_cast<uint8_t*>(&(out_data->k4)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xA4, reinterpret_cast<uint8_t*>(&(out_data->k3)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xA6, reinterpret_cast<uint8_t*>(&(out_data->k2)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xA8, reinterpret_cast<uint8_t*>(&(out_data->k1)), 2);
	if(!ret)
	{
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xAA, reinterpret_cast<uint8_t*>(&(out_data->k0)), 2);
	if(!ret)
	{
		return false;
	}

	return true;
}

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