#pragma once

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

#include <sys/stat.h>
#include <fcntl.h>

#include <mutex>
#include <vector>

class i2c_linux
{
public:

	bool init(const char* dev);

	bool write(const uint8_t dev_addr, uint8_t tx_buf[], const size_t tx_buf_len);
	bool read(const uint8_t dev_addr,  uint8_t* rx_buf,       const size_t rx_buf_len);

	bool write_cmd(const uint8_t dev_addr, uint8_t cmd, uint8_t tx_buf[], const size_t tx_buf_len);
	bool read_cmd(const uint8_t dev_addr,  uint8_t cmd, uint8_t* const rx_buf,  const size_t rx_buf_len);

	bool write_then_read(const uint8_t dev_addr, uint8_t tx_buf[], const size_t tx_buf_len, uint8_t* const rx_buf, const size_t rx_buf_len);
	bool write_then_write(const uint8_t dev_addr, uint8_t tx_buf[], const size_t tx_buf_len, uint8_t* const tx_buf2, const size_t tx_buf2_len);

protected:

	int m_i2c;

	mutable std::mutex m_mutex;
};
