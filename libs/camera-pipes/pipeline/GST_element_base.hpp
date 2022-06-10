/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include <gstreamermm/element.h>
#include <gstreamermm/bin.h>

#include "pipeline/GST_pipeline_base.hpp"

class GST_element_base
{
public:

	GST_element_base()
	{

	}

	virtual ~GST_element_base()
	{

	}

	virtual void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) = 0;
	virtual void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin, const Glib::RefPtr<Gst::Bus>& bus)
	{
		add_to_bin(bin);
	}
	
	// virtual void remove_from_bin(const Glib::RefPtr<Gst::Bin>& bin)
	// {
		
	// }

	virtual bool link_front(const Glib::RefPtr<Gst::Element>& node)
	{
		return false;
	}
	virtual bool link_back(const Glib::RefPtr<Gst::Element>& node)
	{
		return false;
	}

	virtual bool unlink_front(const Glib::RefPtr<Gst::Element>& node)
	{
		return false;
	}
	virtual bool unlink_back(const Glib::RefPtr<Gst::Element>& node)
	{
		return false;
	}

	virtual bool link_front(const Glib::RefPtr<Gst::Pad>& pad)
	{
		return false;
	}
	virtual bool link_back(const Glib::RefPtr<Gst::Pad>& pad)
	{
		return false;
	}

	virtual bool init(const char name[]) = 0;
	
	//todo refactor so you can place new elements in an existing bin
	virtual bool init(const char name[], const Glib::RefPtr<Gst::Bin>& bin)
	{
		return false;
	}

	virtual Glib::RefPtr<Gst::Element> front()
	{
		return Glib::RefPtr<Gst::Element>();
	}
	virtual Glib::RefPtr<Gst::Element> back()
	{
		return Glib::RefPtr<Gst::Element>();
	}

	//todo: actually return a bin
    Glib::RefPtr<Gst::Bin> get_bin() const
    {
		// return m_bin;
		return Glib::RefPtr<Gst::Bin>();
    }

// gst_element_link_pads
// gst_element_link_pads_filtered
// gst_element_link_pads_full


	// Glib::RefPtr<Gst::Pad> get_static_pad(const char name[])
	// {
	// 	return m_element->get_static_pad(name);
	// }

	// Glib::RefPtr<Gst::Element> get_element()
	// {
	// 	return m_element;
	// }

protected:

	
};
