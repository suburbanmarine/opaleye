#include "pipeline/gst_common.hpp"

std::ostream& Gst::operator<<(std::ostream& os, const Gst::State& state)
{
	switch(state)
	{
		// (0) – no pending state.
		case GST_STATE_VOID_PENDING:
		{
			os << "GST_STATE_VOID_PENDING";
			break;
		}
		// (1) – the NULL state or initial state of an element.
		case GST_STATE_NULL:
		{
			os << "GST_STATE_NULL";
			break;
		}
		// (2) – the element is ready to go to PAUSED.
		case GST_STATE_READY:
		{
			os << "GST_STATE_READY";
			break;
		}
		// (3) – the element is PAUSED, it is ready to accept and process data. Sink elements however only accept one buffer and then block.
		case GST_STATE_PAUSED:
		{
			os << "GST_STATE_PAUSED";
			break;
		}
		// (4) – the element is PLAYING, the GstClock is running and the data is flowing.
		case GST_STATE_PLAYING:
		{
			os << "GST_STATE_PLAYING";
			break;
		}
		default:
		{
			os << "UNKNOWN";
			break;
		}
	}
	return os;
}

std::ostream& Gst::operator<<(std::ostream& os, const Glib::RefPtr<Gst::Message>& msg)
{
    os << msg->get_message_type();
    return os;
}
std::ostream& Gst::operator<<(std::ostream& os, const Gst::MessageType& msg)
{
  switch(msg)
  {
    case GST_MESSAGE_UNKNOWN:
    {
    	os << "GST_MESSAGE_UNKNOWN";
    	break;
    }
    case GST_MESSAGE_EOS:
    {
    	os << "GST_MESSAGE_EOS";
    	break;
    }
    case GST_MESSAGE_ERROR:
    {
    	os << "GST_MESSAGE_ERROR";
    	break;
    }
    case GST_MESSAGE_WARNING:
    {
    	os << "GST_MESSAGE_WARNING";
    	break;
    }
    case GST_MESSAGE_INFO:
    {
    	os << "GST_MESSAGE_INFO";
    	break;
    }
    case GST_MESSAGE_TAG:
    {
    	os << "GST_MESSAGE_TAG";
    	break;
    }
    case GST_MESSAGE_BUFFERING:
    {
    	os << "GST_MESSAGE_BUFFERING";
    	break;
    }
    case GST_MESSAGE_STATE_CHANGED:
    {
    	os << "GST_MESSAGE_STATE_CHANGED";
    	break;
    }
    case GST_MESSAGE_STATE_DIRTY:
    {
    	os << "GST_MESSAGE_STATE_DIRTY";
    	break;
    }
    case GST_MESSAGE_STEP_DONE:
    {
    	os << "GST_MESSAGE_STEP_DONE";
    	break;
    }
    case GST_MESSAGE_CLOCK_PROVIDE:
    {
    	os << "GST_MESSAGE_CLOCK_PROVIDE";
    	break;
    }
    case GST_MESSAGE_CLOCK_LOST:
    {
    	os << "GST_MESSAGE_CLOCK_LOST";
    	break;
    }
    case GST_MESSAGE_NEW_CLOCK:
    {
    	os << "GST_MESSAGE_NEW_CLOCK";
    	break;
    }
    case GST_MESSAGE_STRUCTURE_CHANGE:
    {
    	os << "GST_MESSAGE_STRUCTURE_CHANGE";
    	break;
    }
    case GST_MESSAGE_STREAM_STATUS:
    {
    	os << "GST_MESSAGE_STREAM_STATUS";
    	break;
    }
    case GST_MESSAGE_APPLICATION:
    {
    	os << "GST_MESSAGE_APPLICATION";
    	break;
    }
    case GST_MESSAGE_ELEMENT:
    {
    	os << "GST_MESSAGE_ELEMENT";
    	break;
    }
    case GST_MESSAGE_SEGMENT_START:
    {
    	os << "GST_MESSAGE_SEGMENT_START";
    	break;
    }
    case GST_MESSAGE_SEGMENT_DONE:
    {
    	os << "GST_MESSAGE_SEGMENT_DONE";
    	break;
    }
    case GST_MESSAGE_DURATION_CHANGED:
    {
    	os << "GST_MESSAGE_DURATION_CHANGED";
    	break;
    }
    case GST_MESSAGE_LATENCY:
    {
    	os << "GST_MESSAGE_LATENCY";
    	break;
    }
    case GST_MESSAGE_ASYNC_START:
    {
    	os << "GST_MESSAGE_ASYNC_START";
    	break;
    }
    case GST_MESSAGE_ASYNC_DONE:
    {
    	os << "GST_MESSAGE_ASYNC_DONE";
    	break;
    }
    case GST_MESSAGE_REQUEST_STATE:
    {
    	os << "GST_MESSAGE_REQUEST_STATE";
    	break;
    }
    case GST_MESSAGE_STEP_START:
    {
    	os << "GST_MESSAGE_STEP_START";
    	break;
    }
    case GST_MESSAGE_QOS:
    {
    	os << "GST_MESSAGE_QOS";
    	break;
    }
    case GST_MESSAGE_PROGRESS:
    {
    	os << "GST_MESSAGE_PROGRESS";
    	break;
    }
    case GST_MESSAGE_TOC:
    {
    	os << "GST_MESSAGE_TOC";
    	break;
    }
    case GST_MESSAGE_RESET_TIME:
    {
    	os << "GST_MESSAGE_RESET_TIME";
    	break;
    }
    case GST_MESSAGE_STREAM_START:
    {
    	os << "GST_MESSAGE_STREAM_START";
    	break;
    }
    case GST_MESSAGE_NEED_CONTEXT:
    {
    	os << "GST_MESSAGE_NEED_CONTEXT";
    	break;
    }
    case GST_MESSAGE_HAVE_CONTEXT:
    {
    	os << "GST_MESSAGE_HAVE_CONTEXT";
    	break;
    }
    case GST_MESSAGE_EXTENDED:
    {
    	os << "GST_MESSAGE_EXTENDED";
    	break;
    }
    // case GST_MESSAGE_DEVICE_ADDED:
    // {
    	// os << "GST_MESSAGE_DEVICE_ADDED";
    	// break;
    // }
    // case GST_MESSAGE_DEVICE_REMOVED:
    // {
    	// os << "GST_MESSAGE_DEVICE_REMOVED";
    	// break;
    // }
    // case GST_MESSAGE_PROPERTY_NOTIFY:
    // {
    	// os << "GST_MESSAGE_PROPERTY_NOTIFY";
    	// break;
    // }
    // case GST_MESSAGE_STREAM_COLLECTION:
    // {
    	// os << "GST_MESSAGE_STREAM_COLLECTION";
    	// break;
    // }
    // case GST_MESSAGE_STREAMS_SELECTED:
    // {
    	// os << "GST_MESSAGE_STREAMS_SELECTED";
    	// break;
    // }
    // case GST_MESSAGE_REDIRECT:
    // {
    	// os << "GST_MESSAGE_REDIRECT";
    	// break;
    // }
    // case GST_MESSAGE_DEVICE_CHANGED:
    // {
    	// os << "GST_MESSAGE_DEVICE_CHANGED";
    	// break;
    // }
    // case GST_MESSAGE_INSTANT_RATE_REQUEST:
    // {
    	// os << "GST_MESSAGE_INSTANT_RATE_REQUEST";
    	// break;
    // }
    // case GST_MESSAGE_ANY:
    // {
    	// os << "GST_MESSAGE_ANY";
    	// break;
    // }
    default:
    {
    	os << "UNKNOWN";
      break;
    }
  }
  return os;
}
std::ostream& Gst::operator<<(std::ostream& os, const Gst::StateChangeReturn& scr)
{
    switch(scr)
    {
        // (0) – the state change failed
        case GST_STATE_CHANGE_FAILURE:
        {
            os << "GST_STATE_CHANGE_FAILURE";
            break;
        }
        // (1) – the state change succeeded
        case GST_STATE_CHANGE_SUCCESS:
        {
            os << "GST_STATE_CHANGE_SUCCESS";
            break;
        }
        // (2) – the state change will happen asynchronously
        case GST_STATE_CHANGE_ASYNC:
        {
            os << "GST_STATE_CHANGE_ASYNC";
            break;
        }
        // (3) – the state change succeeded but the element cannot produce data in GST_STATE_PAUSED. This typically happens with live sources.
        case GST_STATE_CHANGE_NO_PREROLL:
        {
            os << "GST_STATE_CHANGE_NO_PREROLL";
            break;
        }
        default:
        {
            os << "UNKNOWN";
          break;
        }
    }
    return os;
}
