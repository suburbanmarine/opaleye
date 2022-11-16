#pragma once

#include "v4l2_base.hpp"

#include "opaleye-util/errno_util.hpp"

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

class Alvium_v4l2 : public v4l2_base
{
public:

	Alvium_v4l2();
	virtual ~Alvium_v4l2();

	bool open(const char dev_path[]);
	bool close();

	bool init(const char name[], const uint32_t fcc);

	bool set_free_trigger();
	bool set_sw_trigger();
	bool set_hw_trigger(const Alvium_CSI::v4l2_trigger_source& src, const Alvium_CSI::v4l2_trigger_activation& act);

	bool send_software_trigger();

	// JXR0 - 10-bit/16-bit Bayer RGRG/GBGB
	// JXR2 - 12-bit/16-bit Bayer RGRG/GBGB
	// JXY2 - 12-bit/16-bit Greyscale
	// XR24 - 32-bit BGRX 8-8-8-8

	int get_fd() const
	{
		return m_fd;
	}

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