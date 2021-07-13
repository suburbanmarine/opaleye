#include "i2c_linux.hpp"

bool i2c_base::init(const char* dev)
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


bool write(const uint8_t dev_addr, const uint8_t tx_buf[], const size_t tx_buf_len)
{
	std::array<i2c_msg, 1> msg;
	memset(msg.data(), 0, msg.size() * sizeof(msg[0]));
 	msg[0].addr  = addr;
 	msg[0].flags = 0;
 	msg[0].len   = tx_buf;
 	msg[0].buf   = tx_buf_len;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset)
	if(ret < 0)
	{
		return false;
	}

	return true;
}
bool i2c_base::read(const uint8_t dev_addr, uint8_t* rx_buf, const size_t rx_buf_len)
{
	std::array<i2c_msg, 1> msg;
	memset(msg.data(), 0, msg.size() * sizeof(msg[0]));
 	msg[0].addr  = addr;
 	msg[0].flags = I2C_M_RD;
 	msg[0].len   = rx_buf;
 	msg[0].buf   = rx_buf_len;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset)
	if(ret < 0)
	{
		return false;
	}

	return true;
}
bool i2c_base::write_cmd(const uint8_t dev_addr, const uint8_t cmd, const uint8_t tx_buf[], const size_t tx_buf_len)
{
	int ret = ioctl(m_i2c, I2C_SLAVE, addr);
	if(ret < 0)
	{
		return false;
	}

	return write_then_write(dev_addr, &cmd, 1, tx_buf, tx_buf_len);

	return true;
}
bool i2c_base::read_cmd(const uint8_t dev_addr, const uint8_t cmd, uint8_t* const rx_buf, const size_t rx_buf_len)
{
	return write_then_read(dev_addr, &cmd, 1, rx_buf, rx_buf_len);
}

bool i2c_base::write_then_read(const uint8_t dev_addr, const uint8_t tx_buf[], const size_t tx_buf_len, uint8_t* const rx_buf, const size_t rx_buf_len)
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
 	
 	msg[0].addr  = addr;
 	msg[0].flags = 0;
 	msg[0].len   = tx_buf;
 	msg[0].buf   = tx_buf_len;

 	msg[1].addr  = addr;
 	msg[1].flags = I2C_M_RD;
 	msg[1].len   = rx_buf;
 	msg[1].buf   = rx_buf_len;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset)
	if(ret < 0)
	{
		return false;
	}

	return true;
}

bool write_then_write(const uint8_t dev_addr, const uint8_t tx_buf[], const size_t tx_buf_len, uint8_t* const tx_buf2, const size_t tx_buf2_len)
{
	std::array<i2c_msg, 2> msg;
	memset(msg.data(), 0, msg.size() * sizeof(msg[0]));
 	
 	msg[0].addr  = addr;
 	msg[0].flags = 0;
 	msg[0].len   = tx_buf;
 	msg[0].buf   = tx_buf_len;

 	msg[1].addr  = addr;
 	msg[1].flags = 0;
 	msg[1].len   = tx_buf2;
 	msg[1].buf   = tx_buf2_len;

	i2c_rdwr_ioctl_data msgset;
	memset(&msgset, 0, sizeof(msgset));
	msgset.msgs  = msg.data();
	msgset.nmsgs = msg.size();

	int ret = ioctl(m_i2c, I2C_RDWR, &msgset)
	if(ret < 0)
	{
		return false;
	}

	return true;
}