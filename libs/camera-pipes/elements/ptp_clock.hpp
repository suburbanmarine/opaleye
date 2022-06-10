#pragma once

#include "elements/gst_clock_base.hpp"

#include <array>
#include <string>

gboolean dispatch_GstPtpStatisticsCallback(guint8 domain, const GstStructure * stats,gpointer user_data);

class ptp_clock : public gst_clock_base
{
	public:

	ptp_clock();
	~ptp_clock() override;

	bool init() override;

	gboolean handle_ptp_stat_callback(guint8 domain, const GstStructure * stats);

	protected:

	gulong m_cb_ptr;

	std::array<char, 128> m_ptp_iface0;
	char* m_ptp_iface[2];
};

