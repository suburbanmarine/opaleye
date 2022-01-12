#pragma once

#include <gstreamermm/clock.h>

#include <array>
#include <string>

gboolean dispatch_GstPtpStatisticsCallback(guint8 domain, const GstStructure * stats,gpointer user_data);

class ptp_clock
{
	public:

	ptp_clock();
	~ptp_clock();

	bool init();

	Glib::RefPtr<Gst::Clock> get_clock()
	{
		return m_clock;
	}

	bool wait_for_sync();


	gboolean handle_ptp_stat_callback(guint8 domain, const GstStructure * stats);

	protected:

	gulong m_cb_ptr;

	std::array<char, 128> m_ptp_iface0;
	char* m_ptp_iface[2];
	Glib::RefPtr<Gst::Clock> m_clock;
};

