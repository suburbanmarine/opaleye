#include "Alvium_v4l2.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include <sstream>

#include <cstdio>
#include <cstring>


///
/// Private alvium ioctls
///

#define VIDIOC_TRIGGER_MODE_OFF             _IO('V',  BASE_VIDIOC_PRIVATE + 20)
#define VIDIOC_TRIGGER_MODE_ON              _IO('V',  BASE_VIDIOC_PRIVATE + 21)
#define VIDIOC_S_TRIGGER_ACTIVATION         _IOW('V', BASE_VIDIOC_PRIVATE + 22, int)
#define VIDIOC_G_TRIGGER_ACTIVATION         _IOR('V', BASE_VIDIOC_PRIVATE + 23, int)
#define VIDIOC_S_TRIGGER_SOURCE             _IOW('V', BASE_VIDIOC_PRIVATE + 24, int)
#define VIDIOC_G_TRIGGER_SOURCE             _IOR('V', BASE_VIDIOC_PRIVATE + 25, int)
#define VIDIOC_TRIGGER_SOFTWARE             _IO('V',  BASE_VIDIOC_PRIVATE + 26)

Alvium_v4l2::Alvium_v4l2()
{

}

Alvium_v4l2::~Alvium_v4l2()
{
  
}
bool Alvium_v4l2::init(const char name[], const uint32_t fcc)
{
  return v4l2_base::init(fcc);
}

bool Alvium_v4l2::set_free_trigger()
{
  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_MODE_OFF ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_MODE_OFF failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}
bool Alvium_v4l2::set_sw_trigger()
{
  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_MODE_ON ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_MODE_ON failed: {:s}", m_errno.to_str());
      return false;
  }

  // Set trigger source to software
  int source = (int)Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_SOFTWARE;
  if( m_v4l2_util.ioctl_helper(VIDIOC_S_TRIGGER_SOURCE, &source ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_S_TRIGGER_SOURCE / V4L2_TRIGGER_SOURCE_SOFTWARE failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}
bool Alvium_v4l2::set_hw_trigger(const Alvium_CSI::v4l2_trigger_source& src, const Alvium_CSI::v4l2_trigger_activation& act)
{
  switch(src)
  {
    case Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_LINE0:
    case Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_LINE1:
    case Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_LINE2:
    case Alvium_CSI::v4l2_trigger_source::V4L2_TRIGGER_SOURCE_LINE3:
    {
      break;
    }
    default:
    {
      SPDLOG_ERROR("v4l2_trigger_source not valid");
      return false;
    }
  }

  switch(act)
  {
    case Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_RISING_EDGE:
    case Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_FALLING_EDGE:
    case Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_ANY_EDGE:
    case Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_LEVEL_HIGH:
    case Alvium_CSI::v4l2_trigger_activation::V4L2_TRIGGER_ACTIVATION_LEVEL_LOW:
    {
      break;
    }
    default:
    {
      SPDLOG_ERROR("v4l2_trigger_activation not valid");
      return false;
    }
  }

  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_MODE_ON ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_MODE_ON failed: {:s}", m_errno.to_str());
      return false;
  }

  // Set trigger source
  int source = (int)src;
  if( m_v4l2_util.ioctl_helper(VIDIOC_S_TRIGGER_SOURCE, &source ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_S_TRIGGER_SOURCE / V4L2_TRIGGER_SOURCE_LINE0 failed: {:s}", m_errno.to_str());
      return false;
  }

  // Set trigger activation
  int activation = (int)act;
  if( m_v4l2_util.ioctl_helper(VIDIOC_S_TRIGGER_ACTIVATION, &activation ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_S_TRIGGER_ACTIVATION failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}

bool Alvium_v4l2::send_software_trigger()
{
  // Set trigger mode
  if( m_v4l2_util.ioctl_helper(VIDIOC_TRIGGER_SOFTWARE ) == -1 )
  {
      SPDLOG_ERROR("ioctl VIDIOC_TRIGGER_SOFTWARE failed: {:s}", m_errno.to_str());
      return false;
  }

  return true;
}

