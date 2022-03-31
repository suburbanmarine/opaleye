#include <util/v4l2_metadata.hpp>
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
void v4l2_metadata::v4l2_pix_format_to_json(const v4l2_pix_format& pix, boost::property_tree::ptree* const out_ptree)
{

}
void v4l2_metadata::v4l2_pix_format_mplane_to_json(const v4l2_pix_format_mplane& pix_mp, boost::property_tree::ptree* const out_ptree)
{

}