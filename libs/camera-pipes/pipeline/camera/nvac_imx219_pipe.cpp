/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "nvac_imx219_pipe.hpp"
#include "v4l2_util.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <sys/ioctl.h>

// v4l2-ctl --list-devices
// vi-output, imx219 9-0010 (platform:15c10000.vi:0):
//     /dev/video0
// 
// vi-output, imx219 10-0010 (platform:15c10000.vi:2):
//     /dev/video1


// v4l2-ctl -d /dev/video0 --list-formats-ext
// ioctl: VIDIOC_ENUM_FMT
//     Index       : 0
//     Type        : Video Capture
//     Pixel Format: 'RG10'
//     Name        : 10-bit Bayer RGRG/GBGB
//         Size: Discrete 3264x2464
//             Interval: Discrete 0.048s (21.000 fps)
//         Size: Discrete 3264x1848
//             Interval: Discrete 0.036s (28.000 fps)
//         Size: Discrete 1920x1080
//             Interval: Discrete 0.033s (30.000 fps)
//         Size: Discrete 1640x1232
//             Interval: Discrete 0.033s (30.000 fps)
//         Size: Discrete 1280x720
//             Interval: Discrete 0.017s (60.000 fps)



nvac_imx219_pipe::nvac_imx219_pipe()
{
  
}

void nvac_imx219_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);

  m_bus = bin->get_bus();
}

bool nvac_imx219_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}
bool nvac_imx219_pipe::link_back(const Glib::RefPtr<Gst::Element>& node)
{
  m_out_tee->link(node);
  return true;
}

bool nvac_imx219_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    //source
    m_src = Gst::ElementFactory::create_element("nvarguscamerasrc", name);
    m_src->set_property("do-timestamp", true);
    m_src->set_property("is-live",      true);
    m_src->set_property("sensor-id",    0);
    // m_src->set_property("bufapi-version",    false); // set to use new Buffer API
    
    // exposuretimerange
    // gainrange
    // wbmode
    // saturation
    // aeantibanding
    // exposurecompensation
    // aelock
    // awblock

    // m_src->add_probe(GST_PAD_PROBE_TYPE_IDLE | GST_PAD_PROBE_TYPE_EVENT_BOTH, sigc::mem_fun(&nvac_imx219_pipe::on_pad_probe, this))

    //src caps
    // see https://forums.developer.nvidia.com/t/using-x-raw-memory-nvmm-in-gstreamer-program/42654
    // see https://forums.developer.nvidia.com/t/using-x-raw-memory-nvmm-in-gstreamer-program/42654/9
    // m_src_caps = Gst::Caps::create_from_string((Glib::ustring)"video/x-raw(memory:NVMM),format=(string)NV12,pixel-aspect-ratio(fraction)1/1,framerate=(fraction)30/1,width=(int)1920,height=(int)1080");
    m_src_caps = Gst::Caps::create_from_string("video/x-raw(memory:NVMM), width=(int)1920, height=(int)1080, format=(string)NV12, framerate=(fraction)30/1, pixel-aspect-ratio=(fraction)1/1");
    // m_src_caps = Gst::Caps::create_simple(
    //   "video/x-raw(memory:NVMM)",
    //   "format","NV12",
    //   "pixel-aspect-ratio", Gst::Fraction(1, 1),
    //   "framerate",          Gst::Fraction(30, 1),
    //   "width",              1920,
    //   "height",             1080
    //   );

    m_in_capsfilter = Gst::CapsFilter::create("incaps");
    m_in_capsfilter->property_caps() = m_src_caps;
    // m_in_capsfilter->set_property("caps", m_src_caps);


    m_in_queue     = Gst::Queue::create();
    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    // m_in_queue->property_min_threshold_time()    = 0;
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1 * GST_SECOND;

    //output tee
    m_out_tee = Gst::Tee::create();

    m_bin->add(m_src);
    m_bin->add(m_in_capsfilter);
    m_bin->add(m_in_queue);
    m_bin->add(m_out_tee);
  }

  m_src->link(m_in_capsfilter);
  m_in_capsfilter->link(m_in_queue);
  m_in_queue->link(m_out_tee);
  
  return true;
}

void nvac_imx219_pipe::set_sensor_id(int val)
{
    m_src->set_property("sensor-id", val);
}

bool nvac_imx219_pipe::set_exposure_mode(int32_t val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_AUTO, val);
}
bool nvac_imx219_pipe::get_exposure_mode(int32_t* const val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_AUTO, val);
}
bool nvac_imx219_pipe::set_exposure_value(int32_t val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}
bool nvac_imx219_pipe::get_exposure_value(int32_t* const val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_EXPOSURE_ABSOLUTE, val);
}

bool nvac_imx219_pipe::set_focus_absolute(int32_t val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_FOCUS_ABSOLUTE, val);
}
bool nvac_imx219_pipe::get_focus_absolute(int32_t* const val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_FOCUS_ABSOLUTE, val);
}

bool nvac_imx219_pipe::set_focus_auto(bool val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_FOCUS_AUTO, val);
}
bool nvac_imx219_pipe::get_focus_auto(bool* const val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_FOCUS_AUTO, val);
}

bool nvac_imx219_pipe::set_brightness(int32_t val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_BRIGHTNESS, val);
}
bool nvac_imx219_pipe::get_brightness(int32_t* const val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_BRIGHTNESS, val);
}

bool nvac_imx219_pipe::set_gain(int32_t val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_set(V4L2_CID_GAIN, val);
}
bool nvac_imx219_pipe::get_gain(int32_t* const val)
{
    return false;
	//return m_v4l2_util.v4l2_ctrl_get(V4L2_CID_GAIN, val);
}