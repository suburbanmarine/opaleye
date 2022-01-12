#include"ptp_clock.hpp"

#include <gstreamermm/structure.h>
#include <gstreamer-1.0/gst/net/gstptpclock.h>

#include <spdlog/spdlog.h>

#include <memory>

gboolean dispatch_GstPtpStatisticsCallback(guint8 domain, const GstStructure * stats, gpointer user_data)
{
	return ((ptp_clock*)user_data)->handle_ptp_stat_callback(domain, stats);
}

ptp_clock::ptp_clock()
{
	m_cb_ptr = 0;
}

ptp_clock::~ptp_clock()
{
	if(m_cb_ptr)
	{
		gst_ptp_statistics_callback_remove(m_cb_ptr);
		m_cb_ptr = 0;
	}
}

bool ptp_clock::init()
{
	if( ! gst_ptp_is_supported() )
	{
		SPDLOG_ERROR("gst ptp is not supported");
		return false;
	}

	if( ! gst_ptp_is_initialized() )
	{
		gboolean ret = gst_ptp_init(GST_PTP_CLOCK_ID_NONE, m_ptp_iface);
	}

	GstClock* clk = gst_ptp_clock_new("ptpclk0", 0);
	if(!clk)
	{
		SPDLOG_ERROR("Could not create gst ptp clock");
		return false;
	}

	m_clock = Glib::wrap(clk, false);

	// m_clock

	m_cb_ptr = gst_ptp_statistics_callback_add(&dispatch_GstPtpStatisticsCallback, this, NULL);

	return true;
}

bool ptp_clock::wait_for_sync()
{
	gboolean ret = gst_clock_wait_for_sync(m_clock->gobj(), 1 * GST_SECOND);

	return ret;
}

gboolean ptp_clock::handle_ptp_stat_callback(guint8 domain, const GstStructure * stats)
{
	// GST_PTP_STATISTICS_NEW_DOMAIN_FOUND - G_TYPE_UINT
	// GST_PTP_STATISTICS_BEST_MASTER_CLOCK_SELECTED - G_TYPE_UINT
	// GST_PTP_STATISTICS_PATH_DELAY_MEASURED - G_TYPE_UINT
	// GST_PTP_STATISTICS_TIME_UPDATED - G_TYPE_UINT
	
	// std::shared_ptr<gchar> str(gst_structure_serialize(stats), &g_free);
	std::shared_ptr<gchar> str(gst_structure_to_string(stats), &g_free);
	
	SPDLOG_INFO("ptp_clock: Domain {:d} has stats {:s}: {:s}", int(domain), gst_structure_get_name(stats), str.get());

	return TRUE;
}