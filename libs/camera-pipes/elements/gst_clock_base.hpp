#pragma once

#include <gstreamermm/systemclock.h>

class gst_clock_base
{
	public:

	gst_clock_base()
	{

	}
	virtual ~gst_clock_base()
	{

	}

	virtual bool init() = 0;

	Glib::RefPtr<Gst::Clock> get_clock()
	{
		return m_clock;
	}

	virtual bool wait_for_sync()
	{
		bool ret = m_clock->wait_for_sync(1 * GST_SECOND);

		return ret;
	}

	virtual Gst::ClockTime get_time() const
	{
		return m_clock->get_time();
	}

	protected:

	Glib::RefPtr<Gst::Clock> m_clock;
};

