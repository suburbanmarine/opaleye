#pragma once

#include "opaleye-util/errno_util.hpp"

#include "util/v4l2_util.hpp"
#include "util/v4l2_mmap_buffer.hpp"

#include <vector>
#include <memory>
#include <map>
#include <functional>

namespace Alvium_CSI
{
  enum class v4l2_trigger_source
  {
    V4L2_TRIGGER_SOURCE_SOFTWARE = 0,
    V4L2_TRIGGER_SOURCE_LINE0    = 1, // this seems to be "EXT-GPIO2" aka PWDN
    V4L2_TRIGGER_SOURCE_LINE1    = 2, // this seems to be "EXT-GPIO3" aka MCLK
    V4L2_TRIGGER_SOURCE_LINE2    = 3, // not connected?
    V4L2_TRIGGER_SOURCE_LINE3    = 4  // not connected?
  };
  enum class v4l2_trigger_activation
  {
    V4L2_TRIGGER_ACTIVATION_RISING_EDGE  = 0,
    V4L2_TRIGGER_ACTIVATION_FALLING_EDGE = 1,
    V4L2_TRIGGER_ACTIVATION_ANY_EDGE     = 2,
    V4L2_TRIGGER_ACTIVATION_LEVEL_HIGH   = 3,
    V4L2_TRIGGER_ACTIVATION_LEVEL_LOW    = 4
  };
}

class Alvium_v4l2
{
public:


	typedef std::shared_ptr<v4l2_mmap_buffer>	 MmapFramePtr;
	typedef std::shared_ptr<const v4l2_mmap_buffer>	 ConstMmapFramePtr;
	typedef std::function<void(const ConstMmapFramePtr&)> FrameCallback;

	Alvium_v4l2();
	virtual ~Alvium_v4l2();

	bool open(const char dev_path[]);
	bool close();

	bool init(const char name[], const uint32_t fcc);

	bool start_streaming();
	bool stop_streaming();

	// wait for frame with synchronous callback and V4L2 buffer de-queue / enqueue
	// cb is run in same thread context as caller of wait_for_frame
	// the ConstMmapFramePtr must not have lifetime extended beyond time within the FrameCallback
	bool wait_for_frame(const std::chrono::microseconds& timeout);
	bool wait_for_frame(const std::chrono::microseconds& timeout, const FrameCallback& cb);

	// wait for frame with asynchronous callback and V4L2 buffer de-queue / enqueue
	// causes buffer copy to local buffer
	// bool async_wait_for_frame(const std::chrono::microseconds& timeout);
	// bool async_wait_for_frame(const std::chrono::microseconds& timeout, const FrameCallback& cb);

	bool set_free_trigger();
	bool set_sw_trigger();
	bool set_hw_trigger(const Alvium_CSI::v4l2_trigger_source& src, const Alvium_CSI::v4l2_trigger_activation& act);

	bool send_software_trigger();

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

	static constexpr size_t NUM_BUFFERS = 3;
};