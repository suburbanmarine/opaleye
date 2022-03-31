#pragma once

#include <boost/property_tree/ptree.hpp>

#include <linux/videodev2.h>

class v4l2_metadata
{
public:
	static void v4l2_buffer_to_json(const v4l2_buffer& buf, boost::property_tree::ptree* const out_ptree);
	static void v4l2_timecode_to_json(const v4l2_timecode& tc, boost::property_tree::ptree* const out_ptree);
	static void v4l2_format_to_json(const v4l2_format& fmt, boost::property_tree::ptree* const out_ptree);

	static void v4l2_plane_to_json(const v4l2_plane& plane, boost::property_tree::ptree* const out_ptree);

	static void v4l2_pix_format_to_json(const v4l2_pix_format& pix, boost::property_tree::ptree* const out_ptree);
	static void v4l2_pix_format_mplane_to_json(const v4l2_pix_format_mplane& pix_mp, boost::property_tree::ptree* const out_ptree);
	static void v4l2_plane_pix_format_to_json(const v4l2_plane_pix_format& plane_pix, boost::property_tree::ptree* const out_ptree);
protected:
};
