#include "util/v4l2_metadata.hpp"
#include "util/v4l2_util.hpp"

void v4l2_metadata::v4l2_buffer_to_json(const v4l2_buffer& buf, boost::property_tree::ptree* const out_ptree)
{
	out_ptree->clear();

	out_ptree->put("index",             buf.index);
	out_ptree->put("type",              buf.type);
	out_ptree->put("bytesused",         buf.bytesused);
	out_ptree->put("flags",             buf.flags);
	out_ptree->put("field",             buf.field);

	if(buf.flags & V4L2_BUF_FLAG_TIMECODE)
	{
		boost::property_tree::ptree timecode;
		v4l2_timecode_to_json(buf.timecode, &timecode);
		out_ptree->put_child("timecode", timecode);
	}

	out_ptree->put("timestamp.tv_sec",  buf.timestamp.tv_sec);
	out_ptree->put("timestamp.tv_usec", buf.timestamp.tv_usec);
	out_ptree->put("sequence",          buf.sequence);
	out_ptree->put("memory",            buf.memory);

	if(buf.type == V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE)
	{
		boost::property_tree::ptree planes;
		for(__u32 i = 0; i < buf.length; i++)
		{
			boost::property_tree::ptree plane_i;
			v4l2_plane_to_json(buf.m.planes[i], &plane_i);

			planes.push_back(std::make_pair("", plane_i));
		}

		out_ptree->put_child("m.planes", planes);
	}

	out_ptree->put("length", buf.length);
}
void v4l2_metadata::v4l2_timecode_to_json(const v4l2_timecode& tc, boost::property_tree::ptree* const out_ptree)
{
	out_ptree->clear();

	char const * tc_str = "";

	switch(tc.type)
	{
		case V4L2_TC_TYPE_24FPS:
		{
			tc_str = "24FPS";
			break;
		}
		case V4L2_TC_TYPE_25FPS:
		{
			tc_str = "25FPS";
			break;
		}
		case V4L2_TC_TYPE_30FPS:
		{
			tc_str = "30FPS";
			break;
		}
		case V4L2_TC_TYPE_50FPS:
		{
			tc_str = "50FPS";
			break;
		}
		case V4L2_TC_TYPE_60FPS:
		{
			tc_str = "60FPS";
			break;
		}
		default:
		{
			tc_str = "UNKFPS";
			break;
		}
	}

	out_ptree->put("type",    tc_str);
	out_ptree->put("flags",   tc.flags);
	out_ptree->put("frames",  tc.frames);
	out_ptree->put("seconds", tc.seconds);
	out_ptree->put("minutes", tc.minutes);
	out_ptree->put("hours",   tc.hours);
}
void v4l2_metadata::v4l2_format_to_json(const v4l2_format& fmt, boost::property_tree::ptree* out_ptree)
{
	out_ptree->clear();

	switch(fmt.type)
	{
		case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		{
			boost::property_tree::ptree pix;
			v4l2_pix_format_to_json(fmt.fmt.pix, &pix);
			out_ptree->put_child("fmt.pix", pix);
			break;
		}
		case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
		{
			boost::property_tree::ptree pix_mp;
			v4l2_pix_format_mplane_to_json(fmt.fmt.pix_mp, &pix_mp);
			out_ptree->put_child("fmt.pix_mp", pix_mp);
			break;
		}
		default:
		{
			break;
		}
	}
}

void v4l2_metadata::v4l2_plane_to_json(const v4l2_plane& plane, boost::property_tree::ptree* const out_ptree)
{
	out_ptree->clear();

	out_ptree->put("bytesused", plane.bytesused);
	out_ptree->put("length",    plane.length);
}

void v4l2_metadata::v4l2_pix_format_to_json(const v4l2_pix_format& pix, boost::property_tree::ptree* const out_ptree)
{
	out_ptree->clear();

	out_ptree->put("width",        pix.width);
	out_ptree->put("height",       pix.height);
	out_ptree->put("pixelformat",  v4l2_util::fourcc_to_str(pix.pixelformat));
	out_ptree->put("field",        v4l2_util::v4l2_field_to_str(pix.field));
	out_ptree->put("bytesperline", pix.bytesperline);
	out_ptree->put("sizeimage",    pix.sizeimage);
	out_ptree->put("colorspace",   v4l2_util::v4l2_colorspace_to_str(pix.colorspace));
	out_ptree->put("priv",         pix.priv);
	out_ptree->put("flags",        pix.flags);
	out_ptree->put("ycbcr_enc",    v4l2_util::v4l2_ycbcr_encoding_to_str(pix.ycbcr_enc));
	out_ptree->put("hsv_enc",      v4l2_util::v4l2_hsv_encoding_to_str(pix.hsv_enc));
	out_ptree->put("quantization", v4l2_util::v4l2_quantization_to_str(pix.quantization));
	out_ptree->put("xfer_func",    v4l2_util::v4l2_xfer_func_to_str(pix.xfer_func));
}
void v4l2_metadata::v4l2_pix_format_mplane_to_json(const v4l2_pix_format_mplane& pix_mp, boost::property_tree::ptree* const out_ptree)
{
	out_ptree->clear();

	out_ptree->put("width",        pix_mp.width);
	out_ptree->put("height",       pix_mp.height);
	out_ptree->put("pixelformat",  v4l2_util::fourcc_to_str(pix_mp.pixelformat));
	out_ptree->put("field",        v4l2_util::v4l2_field_to_str(pix_mp.field));
	out_ptree->put("colorspace",   v4l2_util::v4l2_colorspace_to_str(pix_mp.colorspace));

	{
		boost::property_tree::ptree planes;
		for(__u8 i = 0; i < pix_mp.num_planes; i++)
		{
			boost::property_tree::ptree plane_i;
			v4l2_plane_pix_format_to_json(pix_mp.plane_fmt[i], &plane_i);

			planes.push_back(std::make_pair("", plane_i));
		}
		out_ptree->put_child("plane_fmt", planes);
	}

	out_ptree->put("num_planes",   pix_mp.num_planes);
	out_ptree->put("flags",        pix_mp.flags);
	out_ptree->put("ycbcr_enc",    v4l2_util::v4l2_ycbcr_encoding_to_str(pix_mp.ycbcr_enc));
	out_ptree->put("hsv_enc",      v4l2_util::v4l2_hsv_encoding_to_str(pix_mp.hsv_enc));
	out_ptree->put("quantization", v4l2_util::v4l2_quantization_to_str(pix_mp.quantization));
	out_ptree->put("xfer_func",    v4l2_util::v4l2_xfer_func_to_str(pix_mp.xfer_func));
}

void v4l2_metadata::v4l2_plane_pix_format_to_json(const v4l2_plane_pix_format& plane_pix, boost::property_tree::ptree* const out_ptree)
{
	out_ptree->clear();

	out_ptree->put("sizeimage",    plane_pix.sizeimage);
	out_ptree->put("bytesperline", plane_pix.bytesperline);
}
