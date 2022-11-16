#pragma once

#include "opaleye-util/errno_util.hpp"

#include <spdlog/spdlog.h>

#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>

#include <sys/ioctl.h>

#include <cstring>

#include <map>
#include <string>
#include <optional>

class v4l2_ctrl_mgr_base
{
public:
	const std::map<std::string, uint32_t>& get_ctrl_name_map() const
	{
		return m_device_ctrl_by_name;
	}
	const std::map<uint32_t, std::map<int64_t, v4l2_querymenu>>& get_menu_entries() const
	{
		return m_menu_entries;
	}

	std::optional<uint32_t> get_ctrl_id_by_name(const std::string& name) const
	{
		auto it = m_device_ctrl_by_name.find(name);
		if(it == m_device_ctrl_by_name.end())
		{
			return std::optional<uint32_t>();
		}

		return it->second;
	}

	std::optional<v4l2_capability> v4l2_probe_caps(int fd)
	{
		v4l2_capability cap;
		memset(&cap, 0, sizeof(cap));
		int ret = ::ioctl(fd, VIDIOC_QUERYCAP, &cap);
		if(ret < 0)
		{
			SPDLOG_WARN("VIDIOC_QUERYCAP error: {:s}", m_errno.to_str());
			return std::optional<v4l2_capability>();
		}

		return std::optional<v4l2_capability>(cap);
	}

protected:
	// ctrl name -> ctrl id
	std::map<std::string, uint32_t> m_device_ctrl_by_name;
	// ctrl id -> index -> menu_entries
	std::map<uint32_t, std::map<int64_t, v4l2_querymenu>> m_menu_entries;

	errno_util m_errno;
};

template< typename T >
class v4l2_ctrl_mgr_base_T : public v4l2_ctrl_mgr_base
{
public:
	const std::map<uint32_t, T>& get_ctrl_map() const
	{
		return m_device_ctrl;
	}

	std::optional<T> get_ctrl_by_name(const std::string& name) const
	{
		std::optional<uint32_t> id = get_ctrl_id_by_name(name);
		if( ! id.has_value() )
		{
			return std::optional<T>();
		}

		return get_ctrl_by_id(id.value());
	}

	std::optional<T> get_ctrl_by_id(const uint32_t& id) const
	{
		auto it = m_device_ctrl.find(id);
		if(it == m_device_ctrl.end())
		{
			return std::optional<T>();
		}

		return it->second;
	}

	bool v4l2_probe_menu(int fd)
	{
		m_menu_entries.clear();
		for(const auto& ctrl : m_device_ctrl)
		{
			bool is_menu_ctrl = false;
			switch(ctrl.second.type)
			{
				case V4L2_CTRL_TYPE_MENU:
				case V4L2_CTRL_TYPE_INTEGER_MENU:
				{
					is_menu_ctrl = true;
					break;
				}
				default:
				{
					is_menu_ctrl = false;
					break;
				}
			}

			if(is_menu_ctrl)
			{
				std::map<int64_t, v4l2_querymenu>& menu_valid_entries_map_ref = m_menu_entries[ctrl.second.id];

				v4l2_querymenu menu;
				for(int64_t i = ctrl.second.minimum; i <= ctrl.second.maximum; i += ctrl.second.step)
				{
					memset(&menu, 0, sizeof(menu));
					menu.id    = ctrl.second.id;
					menu.index = i;
			
					int ret = ioctl(fd, VIDIOC_QUERYMENU, &menu);
					if(ret < 0)
					{
						//menu entries may be sparse - so just note that this is invalid and keep scanning
						SPDLOG_WARN("VIDIOC_QUERYMENU {:d} {:s}[{:d}]: error: {:s}", 
							ctrl.second.id,
							ctrl.second.name,
							i,
							m_errno.to_str());
						continue;
					}
					else
					{
						menu_valid_entries_map_ref.insert(std::make_pair(i, menu));

						switch(ctrl.second.type)
						{
							case V4L2_CTRL_TYPE_MENU:
							{
								const uint8_t* name = menu.name;
								SPDLOG_DEBUG("VIDIOC_QUERYMENU {:d} {:s}[{:d}]: {:s}",
									ctrl.second.id,
									ctrl.second.name,
									i,
									name);
								break;
							}
							case V4L2_CTRL_TYPE_INTEGER_MENU:
							{
								int64_t value = menu.value;
								SPDLOG_DEBUG("VIDIOC_QUERYMENU {:d} {:s}[{:d}]: {:d}",
									ctrl.second.id,
									ctrl.second.name,
									i,
									value);
								break;
							}
							default:
							{
								SPDLOG_WARN("VIDIOC_QUERYMENU invalid type");
								return false;
							}
						}
					}
				}
			}
		}

		return true;
	}

	bool v4l2_probe_ctrl(int fd, unsigned long ioctl_num)
	{
		m_device_ctrl.clear();
		m_device_ctrl_by_name.clear();
		m_menu_entries.clear();

		std::optional<v4l2_capability> cap = v4l2_probe_caps(fd);
		if( ! cap.has_value() )
		{
			SPDLOG_WARN("v4l2_probe_ctrl v4l2_probe_caps failed");
			return false;
		}


		// V4L2_CID_BASE - V4L2_CID_LASTP1
		// V4L2_CID_PRIVATE_BASE - EINVAL

		T v4l_ctrl;
		memset(&v4l_ctrl, 0, sizeof(v4l_ctrl));
		v4l_ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
		while(0 == ::ioctl(fd, ioctl_num, &v4l_ctrl))
		{
			if( ! v4l_ctrl.flags & V4L2_CTRL_FLAG_DISABLED )
			{
				SPDLOG_DEBUG("v4l2_probe_ctrl {:d} {:s}, {:d}, [{:d}, {:d}]/{:d}",
					v4l_ctrl.id,
					v4l_ctrl.name,
					v4l_ctrl.type,
					v4l_ctrl.minimum,
					v4l_ctrl.maximum,
					v4l_ctrl.step
				);

				const std::string ctrl_name = (const char*)v4l_ctrl.name;

				m_device_ctrl.insert(std::make_pair(v4l_ctrl.id, v4l_ctrl));
				m_device_ctrl_by_name.insert(std::make_pair(ctrl_name, v4l_ctrl.id));

				// if(v4l_ctrl.type == V4L2_CTRL_TYPE_MENU)
				// {
					// probe_menu()
				// }
			}

			v4l_ctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
		}
		if(errno != EINVAL)
		{
			SPDLOG_WARN("v4l2_probe_ctrl error: {:s}", m_errno.to_str());
			return false;
		}

		if( ! v4l2_probe_menu(fd) )
		{
			return false;
		}

		return true;
	}

	bool has_any_ctrl() const
	{
		return m_device_ctrl.empty();
	}

protected:
	// ctrl id -> v4l2_queryctrl / v4l2_query_ext_ctrl
	std::map<uint32_t, T>           m_device_ctrl;
};

class v4l2_ctrl_mgr : public v4l2_ctrl_mgr_base_T<v4l2_queryctrl>
{
public:
	using v4l2_ctrl_mgr_base_T<v4l2_queryctrl>::v4l2_probe_ctrl;

	bool v4l2_probe_ctrl(int fd)
	{
		return this->v4l2_probe_ctrl(fd, VIDIOC_QUERYCTRL);
	}
};

class v4l2_ext_ctrl_mgr : public v4l2_ctrl_mgr_base_T<v4l2_query_ext_ctrl>
{
public:
	using v4l2_ctrl_mgr_base_T<v4l2_query_ext_ctrl>::v4l2_probe_ctrl;

	bool v4l2_probe_ctrl(int fd)
	{
		return this->v4l2_probe_ctrl(fd, VIDIOC_QUERY_EXT_CTRL);
	}
};