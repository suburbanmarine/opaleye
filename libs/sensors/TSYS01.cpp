#include "TSYS01.hpp"

#include <spdlog/spdlog.h>

#include <arpa/inet.h>

#include <thread>

bool TSYS01::init(const std::shared_ptr<i2c_iface>& i2c)
{
	m_i2c = i2c;
	m_dev_addr = 0x77;

	return true;
}

bool TSYS01::sample(uint32_t* out_sample)
{
	std::array<uint8_t, 1> buf = {0x48};
	bool ret = m_i2c->write(m_dev_addr, buf.data(), buf.size());
	if(!ret)
	{
		SPDLOG_WARN("Sample trigger failed");
		return false;
	}

	//wait for conversion
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	std::array<uint8_t, 3> rx_buf;
	ret = m_i2c->read_cmd(m_dev_addr, 0x00, rx_buf.data(), rx_buf.size());
	if(!ret)
	{
		SPDLOG_WARN("Sample download failed");
		return false;
	}

	uint32_t temp_sample = (uint32_t(rx_buf[0]) << 16) | (uint32_t(rx_buf[1]) << 8) | (uint32_t(rx_buf[2]) << 0);
	SPDLOG_DEBUG("Temp sample: {:d}", temp_sample);

	*out_sample = temp_sample;

	return true;
}

bool TSYS01::reset()
{
	std::array<uint8_t, 1> buf = {0x1E};
	bool ret = m_i2c->write(m_dev_addr, buf.data(), buf.size());
	if(!ret)
	{
		SPDLOG_WARN("Reset failed");
		return false;
	}

	return true;
}

bool TSYS01::read_cal_data(CAL_DATA* const out_data)
{
	bool ret = m_i2c->read_cmd(m_dev_addr, 0xA2, reinterpret_cast<uint8_t*>(&(out_data->k4)), 2);
	if(!ret)
	{
		SPDLOG_WARN("Cal A2 failed");
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xA4, reinterpret_cast<uint8_t*>(&(out_data->k3)), 2);
	if(!ret)
	{
		SPDLOG_WARN("Cal A4 failed");
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xA6, reinterpret_cast<uint8_t*>(&(out_data->k2)), 2);
	if(!ret)
	{
		SPDLOG_WARN("Cal A6 failed");
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xA8, reinterpret_cast<uint8_t*>(&(out_data->k1)), 2);
	if(!ret)
	{
		SPDLOG_WARN("Cal A8 failed");
		return false;
	}

	ret = m_i2c->read_cmd(m_dev_addr, 0xAA, reinterpret_cast<uint8_t*>(&(out_data->k0)), 2);
	if(!ret)
	{
		SPDLOG_WARN("Cal AA failed");
		return false;
	}

	out_data->k4 = ntohs(out_data->k4);
	out_data->k3 = ntohs(out_data->k3);
	out_data->k2 = ntohs(out_data->k2);
	out_data->k1 = ntohs(out_data->k1);
	out_data->k0 = ntohs(out_data->k0);

	SPDLOG_DEBUG("Cal data: 0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}, 0x{:02X}", 
		out_data->k4,
		out_data->k3,
		out_data->k2,
		out_data->k1,
		out_data->k0
	);

	return true;
}

double TSYS01::calc_temp(const uint32_t sample, const CAL_DATA& data)
{
	const uint32_t ADC24 = sample;
	const uint32_t ADC16 = ADC24 / uint32_t(256UL);

	const uint64_t ADC16_1 = ADC16;
	const uint64_t ADC16_2 = ADC16_1 * ADC16;
	const uint64_t ADC16_3 = ADC16_2 * ADC16;
	const uint64_t ADC16_4 = ADC16_3 * ADC16;

	double temp = 0.0;
	temp += -2.0 * double(data.k4) * 1e-21 * double(ADC16_4);
	temp +=  4.0 * double(data.k3) * 1e-26 * double(ADC16_3);
	temp += -2.0 * double(data.k2) * 1e-11 * double(ADC16_2);
	temp +=  1.0 * double(data.k1) * 1e-6  * double(ADC16_1);
	temp += -1.5 * double(data.k0) * 1e-2;

	return temp;
}