/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "util/v4l2_util.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <sys/ioctl.h>

#include <cstring>

#include <array>
#include <sstream>
#include <string>

v4l2_util::v4l2_util()
{
	m_v4l2_fd = -1;
}
v4l2_util::~v4l2_util()
{

}

std::string v4l2_util::fourcc_to_str(const uint32_t fcc)
{
	std::stringstream ss;
	ss << char( (fcc & 0x000000FFUL) >> 0 );
	ss << char( (fcc & 0x0000FF00UL) >> 8 );
	ss << char( (fcc & 0x00FF0000UL) >> 16 );
	ss << char( (fcc & 0xFF000000UL) >> 24 );
	return ss.str();
}

const char* v4l2_util::v4l2_field_to_str(const v4l2_field& val)
{
	char const * str = "";
	switch(val)
	{
		case V4L2_FIELD_ANY:
		{
			str = "ANY";
			break;
		}
		case V4L2_FIELD_NONE:
		{
			str = "NONE";
			break;
		}
		case V4L2_FIELD_TOP:
		{
			str = "TOP";
			break;
		}
		case V4L2_FIELD_BOTTOM:
		{
			str = "BOTTOM";
			break;
		}
		case V4L2_FIELD_INTERLACED:
		{
			str = "INTERLACED";
			break;
		}
		case V4L2_FIELD_SEQ_TB:
		{
			str = "SEQ_TB";
			break;
		}
		case V4L2_FIELD_SEQ_BT:
		{
			str = "SEQ_BT";
			break;
		}
		case V4L2_FIELD_ALTERNATE:
		{
			str = "ALTERNATE";
			break;
		}
		case V4L2_FIELD_INTERLACED_TB:
		{
			str = "INTERLACED_TB";
			break;
		}
		case V4L2_FIELD_INTERLACED_BT:
		{
			str = "INTERLACED_BT";
			break;
		}
		default:
		{
			str = "UNKNOWN";
			break;
		}
	}

	return str;
}
const char* v4l2_util::v4l2_colorspace_to_str(const v4l2_colorspace& val)
{
	char const * str = "";
	switch(val)
	{
		case V4L2_COLORSPACE_DEFAULT:
		{
			str = "DEFAULT";
			break;
		}
		case V4L2_COLORSPACE_SMPTE170M:
		{
			str = "SMPTE170M";
			break;
		}
		case V4L2_COLORSPACE_REC709:
		{
			str = "REC709";
			break;
		}
		case V4L2_COLORSPACE_SRGB:
		{
			str = "SRGB";
			break;
		}
		case V4L2_COLORSPACE_ADOBERGB:
		{
			str = "ADOBERGB";
			break;
		}
		case V4L2_COLORSPACE_BT2020:
		{
			str = "BT2020";
			break;
		}
		case V4L2_COLORSPACE_DCI_P3:
		{
			str = "DCI_P3";
			break;
		}
		case V4L2_COLORSPACE_SMPTE240M:
		{
			str = "SMPTE240M";
			break;
		}
		case V4L2_COLORSPACE_470_SYSTEM_M:
		{
			str = "470_SYSTEM_M";
			break;
		}
		case V4L2_COLORSPACE_470_SYSTEM_BG:
		{
			str = "470_SYSTEM_BG";
			break;
		}
		case V4L2_COLORSPACE_JPEG:
		{
			str = "JPEG";
			break;
		}
		case V4L2_COLORSPACE_RAW:
		{
			str = "RAW";
			break;
		}
		default:
		{
			str = "UNKNOWN";
			break;
		}
	}

	return str;
}
const char* v4l2_util::v4l2_ycbcr_encoding_to_str(const v4l2_ycbcr_encoding& val)
{
	char const * str = "";
	switch(val)
	{
		case V4L2_YCBCR_ENC_DEFAULT:
		{
			str = "DEFAULT";
			break;
		}
		case V4L2_YCBCR_ENC_601:
		{
			str = "601";
			break;
		}
		case V4L2_YCBCR_ENC_709:
		{
			str = "709";
			break;
		}
		case V4L2_YCBCR_ENC_XV601:
		{
			str = "XV601";
			break;
		}
		case V4L2_YCBCR_ENC_XV709:
		{
			str = "XV709";
			break;
		}
		case V4L2_YCBCR_ENC_BT2020:
		{
			str = "BT2020";
			break;
		}
		case V4L2_YCBCR_ENC_BT2020_CONST_LUM:
		{
			str = "BT2020_CONST_LUM";
			break;
		}
		case V4L2_YCBCR_ENC_SMPTE240M:
		{
			str = "SMPTE_240M";
			break;
		}
		default:
		{
			str = "UNKNOWN";
			break;
		}
	}

	return str;
}
const char* v4l2_util::v4l2_hsv_encoding_to_str(const v4l2_hsv_encoding& val)
{
	char const * str = "";
	switch(val)
	{
		case V4L2_HSV_ENC_180:
		{
			str = "180";
			break;
		}
		case V4L2_HSV_ENC_256:
		{
			str = "256";
			break;
		}
		default:
		{
			str = "UNKNOWN";
			break;
		}
	}

	return str;
}
const char* v4l2_util::v4l2_quantization_to_str(const v4l2_quantization& val)
{
	char const * str = "";
	switch(val)
	{
		case V4L2_QUANTIZATION_DEFAULT:
		{
			str = "DEFAULT";
			break;
		}
		case V4L2_QUANTIZATION_FULL_RANGE:
		{
			str = "FULL_RANGE";
			break;
		}
		case V4L2_QUANTIZATION_LIM_RANGE:
		{
			str = "LIM_RANGE";
			break;
		}
		default:
		{
			str = "UNKNOWN";
			break;
		}
	}

	return str;
}
const char* v4l2_util::v4l2_xfer_func_to_str(const v4l2_xfer_func& val)
{
	char const * str = "";
	switch(val)
	{
		case V4L2_XFER_FUNC_DEFAULT:
		{
			str = "DEFAULT";
			break;
		}
		case V4L2_XFER_FUNC_709:
		{
			str = "709";
			break;
		}
		case V4L2_XFER_FUNC_SRGB:
		{
			str = "SRGB";
			break;
		}
		case V4L2_XFER_FUNC_ADOBERGB:
		{
			str = "ADOBERGB";
			break;
		}
		case V4L2_XFER_FUNC_SMPTE240M:
		{
			str = "SMPTE240M";
			break;
		}
		case V4L2_XFER_FUNC_NONE:
		{
			str = "NONE";
			break;
		}
		case V4L2_XFER_FUNC_DCI_P3:
		{
			str = "DCI_P3";
			break;
		}
		case V4L2_XFER_FUNC_SMPTE2084:
		{
			str = "SMPTE2084";
			break;
		}
		default:
		{
			str = "UNKNOWN";
			break;
		}
	}

	return str;
}

int v4l2_util::ioctl_helper(int req)
{
  int ret = 0;

  do
  {
    ret = ioctl(m_v4l2_fd, req);
  } while((ret == -1) && (errno == EINTR));

  return ret;
}

int v4l2_util::ioctl_helper(int req, void* arg)
{
  int ret = 0;

  do
  {
    ret = ioctl(m_v4l2_fd, req, arg);
  } while((ret == -1) && (errno == EINTR));

  return ret;
}

bool v4l2_util::enum_format_descs(const v4l2_buf_type buf_type)
{
	m_fmt_descs.clear();

	for(__u32 i = 0; true; i++)
	{
		v4l2_fmtdesc fmt;
		memset(&fmt, 0, sizeof(fmt));
		fmt.type = buf_type;
		
		fmt.index = i;

		int ret = ioctl_helper(VIDIOC_ENUM_FMT, &fmt);
		if(ret == -1)
		{
			if(errno == EINVAL)
			{
				break;
			}
			else
			{
				SPDLOG_ERROR("ioctl VIDIOC_ENUM_FMT had error: {:d} - {:s}", errno, m_errno.to_str());
				m_fmt_descs.clear();
				return false;
			}
		}

		m_fmt_descs.push_back(fmt);
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const bool val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;
	ctrl.value = (val) ? 1 : 0;

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const int32_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;
	ctrl.value = val;

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const int64_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;
	ctrl.value64 = val;

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_set(uint32_t id, const uint8_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 1;

	std::array<uint8_t, 1> arr = {val};
	ctrl.p_u8  = arr.data();

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;	
}
bool v4l2_util::v4l2_ctrl_set(uint32_t id, const uint16_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 2;

	std::array<uint16_t, 1> arr = {val};
	ctrl.p_u16 = arr.data();

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}
bool v4l2_util::v4l2_ctrl_set(uint32_t id, const uint32_t val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 4;
	
	std::array<uint32_t, 1> arr = {val};
	ctrl.p_u32 = arr.data();

	if( ! v4l2_ctrl_set(&ctrl) )
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, bool* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = (ctrl.value) ? true : false;
	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, int32_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.value;
	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, int64_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 0;

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = ctrl.value64;
	return true;
}

bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint8_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 1;

	std::array<uint8_t, 1> arr = {0};
	ctrl.p_u8  = arr.data();

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = arr[0];
	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint16_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 2;

	std::array<uint16_t, 1> arr = {0};
	ctrl.p_u16 = arr.data();

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = arr[0];
	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t id, uint32_t which, uint32_t* const out_val)
{
	v4l2_ext_control ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.id    = id;
	ctrl.size  = 4;

	std::array<uint32_t, 1> arr = {0};
	ctrl.p_u32 = arr.data();

	if( ! v4l2_ctrl_get(which, &ctrl) )
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	*out_val = arr[0];
	return true;
}

bool v4l2_util::v4l2_ctrl_set(v4l2_ext_control* const ctrl)
{
	v4l2_ext_controls ctrls;
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.which      = V4L2_CTRL_WHICH_CUR_VAL;
	ctrls.ctrl_class = 0; // V4L2_CTRL_ID2CLASS(ctrl->id);
	ctrls.count      = 1;
	ctrls.controls   = ctrl;

	int ret = ioctl(m_v4l2_fd, VIDIOC_S_EXT_CTRLS, &ctrls);	
	if(ret < 0)
	{
		SPDLOG_WARN("VIDIOC_S_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}
bool v4l2_util::v4l2_ctrl_get(uint32_t which, v4l2_ext_control* const ctrl)
{
	v4l2_ext_controls ctrls;
	memset(&ctrls, 0, sizeof(ctrls));
	ctrls.which      = which;
	ctrls.ctrl_class = 0; // V4L2_CTRL_ID2CLASS(ctrl->id);
	ctrls.count      = 1;
	ctrls.controls   = ctrl;

	int ret = ioctl(m_v4l2_fd, VIDIOC_G_EXT_CTRLS, &ctrls);	
	if(ret < 0)
	{
		SPDLOG_WARN("VIDIOC_G_EXT_CTRLS error: {:s}", m_errno.to_str());
		return false;
	}

	return true;
}

std::shared_ptr<v4l2_util::JsonDoc> v4l2_util::get_property_description()
{
	using namespace rapidjson;

	rapidjson::CrtAllocator valueAlloc;
	rapidjson::CrtAllocator parseAlloc;

	std::shared_ptr<JsonDoc> doc = std::make_shared<JsonDoc>(&valueAlloc, 16*1024, &parseAlloc);
	doc->SetObject();

	JsonValue ctrl_desc_array;
	ctrl_desc_array.SetArray();
	
	JsonValue ext_ctrl_desc_array;
	ext_ctrl_desc_array.SetArray();

	if(m_v4l2_ext_ctrl.has_any_ctrl())
	{
		for(const auto& ext_ctrl : m_v4l2_ext_ctrl.get_ctrl_map())
		{
			JsonValue ext_ctrl_desc(rapidjson::kObjectType);
			ext_ctrl_to_json(ext_ctrl.second, ext_ctrl_desc, doc);

			ext_ctrl_desc_array.GetArray().PushBack(ext_ctrl_desc, doc->GetAllocator());
		}
	}
	else if(m_v4l2_ctrl.has_any_ctrl())
	{
		for(const auto& ctrl : m_v4l2_ctrl.get_ctrl_map())
		{
			JsonValue ctrl_desc(rapidjson::kObjectType);

			ctrl_to_json(ctrl.second, ctrl_desc, doc);

			ctrl_desc_array.GetArray().PushBack(ctrl_desc, doc->GetAllocator());
		}
	}

	doc->AddMember("ctrl",     ctrl_desc_array,     doc->GetAllocator());
	doc->AddMember("ext_ctrl", ext_ctrl_desc_array, doc->GetAllocator());

	{
		rapidjson::StringBuffer buf;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
		doc->Accept(writer);

		SPDLOG_INFO("doc: {:s}", buf.GetString());
	}

	return doc;
}

std::optional<v4l2_buf_type> v4l2_util::query_cap()
{
	v4l2_capability cap;
	std::optional<v4l2_buf_type> buffer_type;

	int ret = ioctl_helper(VIDIOC_QUERYCAP, &cap);
  if(ret == -1)
  {
    if(errno == EINVAL)
    {
      SPDLOG_ERROR("Device {:s} is not a V4L2 device");
      return buffer_type;
    }
    else
    {
      SPDLOG_ERROR("ioctl VIDIOC_QUERYCAP failed: {:s}", m_errno.to_str());
      return buffer_type;
    }
  }

  if( ! (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE) || (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE) )
  {
      SPDLOG_ERROR("Device {:s} is not a video capture device");
      return buffer_type;
  }

  if( ! (cap.capabilities & V4L2_CAP_STREAMING)  )
  {
      SPDLOG_ERROR("Device {:s} does not support streaming i/o");
      return buffer_type;
  }

  if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
  {
    enum_format_descs(V4L2_BUF_TYPE_VIDEO_CAPTURE);
    buffer_type = (v4l2_buf_type)V4L2_CAP_VIDEO_CAPTURE;
  }
  else if(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE_MPLANE)
  {
    enum_format_descs(V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE);
    buffer_type = (v4l2_buf_type)V4L2_CAP_VIDEO_CAPTURE_MPLANE;
  }
  else
  {
    SPDLOG_ERROR("Discovered unexpected VIDIOC_QUERYCAP capabilities: {:d}", cap.capabilities);
    return buffer_type;
  }

  return buffer_type;
}

bool v4l2_util::ctrl_to_json(const v4l2_queryctrl& ctrl, JsonValue& out_json, JsonDocPtr& doc)
{
	return false;
}
bool v4l2_util::ext_ctrl_to_json(const v4l2_query_ext_ctrl& ctrl, JsonValue& out_json, JsonDocPtr& doc)
{
	out_json.AddMember<uint32_t>("id",   ctrl.id,   doc->GetAllocator());
	out_json.AddMember<uint32_t>("type", ctrl.type, doc->GetAllocator());

	{
		JsonValue params_name;
		const char* msg = ctrl.name;
		params_name.SetString(msg, strlen(msg), doc->GetAllocator());
		out_json.AddMember("name", params_name, doc->GetAllocator());
	}

	switch(ctrl.type)
	{
		case V4L2_CTRL_TYPE_INTEGER:
		{
			int32_t value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember<int32_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<int64_t>("default_value", ctrl.default_value, doc->GetAllocator());
			out_json.AddMember<int64_t>("minimum",       ctrl.minimum,       doc->GetAllocator());
			out_json.AddMember<int64_t>("maximum",       ctrl.maximum,       doc->GetAllocator());
			out_json.AddMember<uint32_t>("step",         ctrl.step,          doc->GetAllocator());

			break;
		}
		case V4L2_CTRL_TYPE_BOOLEAN:
		{
			bool value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember("value", value, doc->GetAllocator());
			out_json.AddMember<int64_t>("default_value", ctrl.default_value, doc->GetAllocator());

			break;
		}
		case V4L2_CTRL_TYPE_MENU:
		{
			JsonValue valid_params;
			valid_params.SetArray();

			int32_t value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember<int32_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<int64_t>("default_value", ctrl.default_value, doc->GetAllocator());

			auto it = m_v4l2_ext_ctrl.get_menu_entries().find(ctrl.id);
			for(const auto& menu_entry : it->second) // for each index
			{
				JsonValue params;
				params.SetObject();

				JsonValue params_name;
				const char* msg = (const char*)menu_entry.second.name;
				params_name.SetString(msg, strlen(msg), doc->GetAllocator());

				params.AddMember("name", params_name, doc->GetAllocator());
				params.AddMember("index", menu_entry.second.index, doc->GetAllocator());

				valid_params.PushBack(params, doc->GetAllocator());
			}

			out_json.AddMember("enum", valid_params, doc->GetAllocator());

			break;
		}
		case V4L2_CTRL_TYPE_BUTTON:
		{
			break;
		}
		case V4L2_CTRL_TYPE_INTEGER64:
		{
			int64_t value;
			v4l2_ctrl_get(ctrl.id, &value);


			out_json.AddMember<int64_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<int64_t>("default_value", ctrl.default_value, doc->GetAllocator());
			out_json.AddMember<int64_t>("minimum",       ctrl.minimum,       doc->GetAllocator());
			out_json.AddMember<int64_t>("maximum",       ctrl.maximum,       doc->GetAllocator());
			out_json.AddMember<uint64_t>("step",         ctrl.step,          doc->GetAllocator());

			break;
		}
		case V4L2_CTRL_TYPE_CTRL_CLASS:
		{
			break;
		}
		case V4L2_CTRL_TYPE_STRING:
		{
			break;
		}
		case V4L2_CTRL_TYPE_BITMASK:
		{
			int32_t value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember<uint32_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<uint32_t>("default_value", ctrl.default_value, doc->GetAllocator());
			out_json.AddMember<uint32_t>("minimum",       ctrl.minimum,       doc->GetAllocator());
			out_json.AddMember<uint32_t>("maximum",       ctrl.maximum,       doc->GetAllocator());
			break;
		}
		case V4L2_CTRL_TYPE_INTEGER_MENU:
		{
			JsonValue valid_params;
			valid_params.SetArray();

			int32_t value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember<int32_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<int64_t>("default_value", ctrl.default_value, doc->GetAllocator());

			auto it = m_v4l2_ext_ctrl.get_menu_entries().find(ctrl.id);
			for(const auto& menu_entry : it->second)
			{
				JsonValue params;
				params.SetObject();

				JsonValue params_name;
				const char* msg = (const char*)menu_entry.second.name;
				params_name.SetString(msg, strlen(msg), doc->GetAllocator());

				params.AddMember("name", params_name, doc->GetAllocator());
				params.AddMember("index", menu_entry.second.index, doc->GetAllocator());

				valid_params.PushBack(params, doc->GetAllocator());
			}

			out_json.AddMember("enum", valid_params, doc->GetAllocator());
			break;
		}
		case V4L2_CTRL_TYPE_U8:
		{
			uint8_t value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember<uint8_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<uint8_t>("default_value", ctrl.default_value, doc->GetAllocator());
			out_json.AddMember<uint8_t>("minimum",       ctrl.minimum,       doc->GetAllocator());
			out_json.AddMember<uint8_t>("maximum",       ctrl.maximum,       doc->GetAllocator());
			out_json.AddMember<uint8_t>("step",          ctrl.step,          doc->GetAllocator());
			break;
		}
		case V4L2_CTRL_TYPE_U16:
		{
			uint16_t value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember<uint16_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<uint16_t>("default_value", ctrl.default_value, doc->GetAllocator());
			out_json.AddMember<uint16_t>("minimum",       ctrl.minimum,       doc->GetAllocator());
			out_json.AddMember<uint16_t>("maximum",       ctrl.maximum,       doc->GetAllocator());
			out_json.AddMember<uint16_t>("step",          ctrl.step,          doc->GetAllocator());
			break;
		}
		case V4L2_CTRL_TYPE_U32:
		{
			uint32_t value;
			v4l2_ctrl_get(ctrl.id, &value);

			out_json.AddMember<uint32_t>("value",         value,              doc->GetAllocator());
			out_json.AddMember<uint32_t>("default_value", ctrl.default_value, doc->GetAllocator());
			out_json.AddMember<uint32_t>("minimum",       ctrl.minimum,       doc->GetAllocator());
			out_json.AddMember<uint32_t>("maximum",       ctrl.maximum,       doc->GetAllocator());
			out_json.AddMember<uint32_t>("step",          ctrl.step,          doc->GetAllocator());
			break;
		}
		default:
		{
			SPDLOG_ERROR("Unknown type");
			return false;
		}
	}

	return true;
}