/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "pipeline/GST_element_base.hpp"

#include <gstreamermm/bin.h>
#include <gstreamermm/queue.h>
#include <gstreamermm/ghostpad.h>

#include <set>
#include <mutex>
#include <condition_variable>
#include <utility>
#include <string>

class multiudpsink_pipe : public GST_element_base
{
public:

  typedef std::pair<std::string, uint16_t> ConnType;

  multiudpsink_pipe();
  ~multiudpsink_pipe() override;

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  bool link_front(const Glib::RefPtr<Gst::Pad>& pad) override;

  bool init(const char name[]) override;

  Glib::RefPtr<Gst::Element> front() override
  {
    return m_in_queue;
  }

  bool add_client(const std::string& host, uint16_t port);
  bool remove_client(const std::string& host, uint16_t port);
  bool remove_all_clients(const std::string& host, uint16_t port);

  bool has_connection(const ConnType& conn) const
  {
    std::unique_lock<std::mutex> lock(m_active_conn_mutex);
    return has_connection_no_lock(conn);
  }

protected:

  static void dispatch_client_added(GstElement * gstmultiudpsink, gchararray host, gint port, gpointer udata)
  {
    reinterpret_cast<multiudpsink_pipe*>(udata)->handle_client_added(host, port);
  }
  void handle_client_added(gchararray host, gint port);
  static void dispatch_client_removed(GstElement * gstmultiudpsink, gchararray host, gint port, gpointer udata)
  {
    reinterpret_cast<multiudpsink_pipe*>(udata)->handle_client_removed(host, port);
  }
  void handle_client_removed(gchararray host, gint port);

  bool has_connection_no_lock(const ConnType& conn) const
  {
    return m_active_conn.find(conn) != m_active_conn.end();
  }

  Glib::RefPtr<Gst::Bin>        m_bin;
  Glib::RefPtr<Gst::GhostPad>   m_bin_sink;

  Glib::RefPtr<Gst::Queue>      m_in_queue;
  Glib::RefPtr<Gst::Element>    m_multiudpsink;
  Glib::RefPtr<Glib::Object>    m_multiudpsink_obj;

  std::list<sigc::connection> signal_handlers;

  std::set<ConnType>      m_active_conn;
  mutable std::mutex      m_active_conn_mutex;
  std::condition_variable m_active_conn_cv;

  static const Glib::SignalProxyInfo client_added_signal_info;
  static const Glib::SignalProxyInfo client_removed_signal_info;

  // Glib::SignalProxy<void(gchararray host, gint port)> signal_client_added()
  // {
  //   return Glib::SignalProxy<void(gchararray host, gint port)>(m_multiudpsink_obj.operator->(), &client_added_signal_info);
  // }

  // Glib::SignalProxy<void(gchararray host, gint port)> signal_client_removed()
  // {
  //   return Glib::SignalProxy<void(gchararray host, gint port)>(m_multiudpsink_obj.operator->(), &client_added_signal_info);
  // }
};
