#pragma once

#include <gstreamermm/message.h>
#include <gstreamermm/element.h>

#include <ostream>
namespace Gst
{
	std::ostream& operator<<(std::ostream& os, const Gst::State& state);
	std::ostream& operator<<(std::ostream& os, const Glib::RefPtr<Gst::Message>& msg);
	std::ostream& operator<<(std::ostream& os, const Gst::MessageType& msg);
	std::ostream& operator<<(std::ostream& os, const Gst::StateChangeReturn& scr);
}

// template <>
// struct fmt::formatter<point>
// {

// }