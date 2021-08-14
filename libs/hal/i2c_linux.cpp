/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "i2c_linux.hpp"

#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>

#include <spdlog/spdlog.h>

i2c_linux::i2c_linux()
{
	m_i2c = -1;
}

i2c_linux::~i2c_linux()
{
	if(m_i2c >= 0)
	{
		close(m_i2c);
		m_i2c = -1;
	}
}

bool i2c_linux::init(const char* dev)
{
	if(m_i2c >= 0)
	{
		return false;
	}

	// "/dev/i2c-0"
	m_i2c = open(dev, O_RDWR);
	if(m_i2c < 0)
	{
		return false;
	}

	return true;
}

bool i2c_linux::write(const uint8_t dev_addr, uint8_t tx_buf[], const size_t tx_buf_len)
{
	std::array<i2c_msg, 1> msg;
	memset(msg.data(), 0, msg.size() * sizeof(msg[0]));
 	msg[0].addr  = dev_addr;
 	msg[0].flags = 0;
 	msg[0].len   = tx_buf_len;
 	msg[0].buf   = tx_buf;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset);
	if(ret < 0)
	{
		SPDLOG_WARN("ioctl failed: {:d}", errno);
		return false;
	}

	return true;
}
bool i2c_linux::read(const uint8_t dev_addr, uint8_t* rx_buf, const size_t rx_buf_len)
{
	std::array<i2c_msg, 1> msg;
	memset(msg.data(), 0, msg.size() * sizeof(msg[0]));
 	msg[0].addr  = dev_addr;
 	msg[0].flags = I2C_M_RD;
 	msg[0].len   = rx_buf_len;
 	msg[0].buf   = rx_buf;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset);
	if(ret < 0)
	{
		SPDLOG_WARN("ioctl failed: {:d}", errno);
		return false;
	}

	return true;
}
bool i2c_linux::write_cmd(const uint8_t dev_addr, uint8_t cmd, uint8_t tx_buf[], const size_t tx_buf_len)
{
	return write_then_write(dev_addr, &cmd, 1, tx_buf, tx_buf_len);
}
bool i2c_linux::read_cmd(const uint8_t dev_addr, uint8_t cmd, uint8_t* const rx_buf, const size_t rx_buf_len)
{
	return write_then_read(dev_addr, &cmd, 1, rx_buf, rx_buf_len);
}

bool i2c_linux::write_then_read(const uint8_t dev_addr, uint8_t tx_buf[], const size_t tx_buf_len, uint8_t* const rx_buf, const size_t rx_buf_len)
{
	// https://www.kernel.org/doc/Documentation/i2c/i2c-protocol
	// Flags:
	// I2C_M_TEN
	// I2C_M_RD
	// I2C_M_NOSTART
	// I2C_M_REV_DIR_ADDR
	// I2C_M_IGNORE_NAK
	// I2C_M_NO_RD_ACK
	// I2C_M_RECV_LEN

	std::array<i2c_msg, 2> msg;
	memset(msg.data(), 0, msg.size() * sizeof(msg[0]));
 	
 	msg[0].addr  = dev_addr;
 	msg[0].flags = 0;
 	msg[0].len   = tx_buf_len;
 	msg[0].buf   = tx_buf;

 	msg[1].addr  = dev_addr;
 	msg[1].flags = I2C_M_RD;
 	msg[1].len   = rx_buf_len;
 	msg[1].buf   = rx_buf;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset);
	if(ret < 0)
	{
		SPDLOG_WARN("ioctl failed: {:d}", errno);
		return false;
	}

	return true;
}

bool i2c_linux::write_then_write(const uint8_t dev_addr, uint8_t tx_buf[], const size_t tx_buf_len, uint8_t* const tx_buf2, const size_t tx_buf2_len)
{
	std::array<i2c_msg, 2> msg;
	memset(msg.data(), 0, msg.size() * sizeof(msg[0]));
 	
 	msg[0].addr  = dev_addr;
 	msg[0].flags = 0;
 	msg[0].len   = tx_buf_len;
 	msg[0].buf   = tx_buf;

 	msg[1].addr  = dev_addr;
 	msg[1].flags = 0;
 	msg[1].len   = tx_buf2_len;
 	msg[1].buf   = tx_buf2;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset);
	if(ret < 0)
	{
		SPDLOG_WARN("ioctl failed: {:d}", errno);
		return false;
	}

	return true;
}