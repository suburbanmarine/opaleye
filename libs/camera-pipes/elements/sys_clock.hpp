#pragma once

#include <gstreamermm/systemclock.h>

#include <array>
#include <string>

class sys_clock
{
	public:

	sys_clock();
	~sys_clock();

	bool init();

	Glib::RefPtr<Gst::Clock> get_clock()
	{
		return m_clock;
	}

	bool wait_for_sync();

	Gst::ClockTime get_time() const;

	protected:

	Glib::RefPtr<Gst::Clock> m_clock;
};

