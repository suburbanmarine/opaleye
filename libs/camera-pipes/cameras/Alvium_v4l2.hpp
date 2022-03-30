#pragma once

#include "opaleye-util/errno_util.hpp"

#include "util/v4l2_util.hpp"

#include <vector>
#include <memory>
#include <map>

class Alvium_v4l2
{
public:
	Alvium_v4l2();
	virtual ~Alvium_v4l2();

	void reset();

	bool open(const char dev_path[]);
	bool close();

	bool init(const char name[]);

	bool start_streaming();
	bool stop_streaming();
	bool wait_for_frame();

protected:
	errno_util m_errno;

	std::string dev_path;
	std::string dev_name;
	int m_fd;

	v4l2_util m_v4l2_util;

	v4l2_capability m_cap;
	v4l2_buf_type m_buffer_type;
	// std::vector<uint8_t> m_frame_buf;

	std::map<uint32_t, std::shared_ptr<v4l2_mmap_buffer>> m_buf_by_idx;
	std::map<void*,    std::shared_ptr<v4l2_mmap_buffer>> m_buf_by_ptr;
};