#include "Thumbnail_sw2_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

Thumbnail_sw2_pipe::Thumbnail_sw2_pipe()
{

}
Thumbnail_sw2_pipe::~Thumbnail_sw2_pipe()
{

}

void Thumbnail_sw2_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}

bool Thumbnail_sw2_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
}

bool Thumbnail_sw2_pipe::init(const char name[])
{
  //init our internal bin and elements
  {
    m_bin = Gst::Bin::create(fmt::format("{:s}-bin", name).c_str());

    //queue
    m_in_queue = Gst::Queue::create();
    // m_in_queue->set_property("leaky", Gst::QUEUE_LEAK_DOWNSTREAM);
    // m_in_queue->property_min_threshold_time()    = 0;
    // m_in_queue->property_min_threshold_buffers() = 0;
    // m_in_queue->property_min_threshold_bytes()   = 0;
    m_in_queue->property_max_size_buffers()      = 0;
    m_in_queue->property_max_size_bytes()        = 0;
    m_in_queue->property_max_size_time()         = 1000 * GST_MSECOND;

    //out caps
    m_appsink_caps = Gst::Caps::create_simple(
      "image/jpeg",
      "pixel-aspect-ratio", Gst::Fraction(1, 1)
      // "format","JPG",
      // "width",              3840,
      // "height",             2160
      );

    m_appsink = Gst::AppSink::create();
    // m_appsink->property_caps().set_value(m_appsink_caps);
    m_appsink->property_emit_signals() = true;
    m_appsink->property_drop()         = true;
    m_appsink->property_max_buffers()  = 1;
    m_appsink->property_sync()         = false;

    m_appsink->signal_new_sample().connect(
      [this]()
      {
        handle_new_sample();
        return Gst::FLOW_OK;
      }
    );

    m_bin->add(m_in_queue);
    m_bin->add(m_appsink);
  }

  m_in_queue->link(m_appsink);

  m_frame_buffer = std::make_shared<std::vector<uint8_t>>();
  m_frame_buffer->reserve(3480UL*2160UL*3UL);

  m_thumb_jpeg_buffer_front = std::make_shared<std::vector<uint8_t>>();
  m_thumb_jpeg_buffer_front->reserve(640UL*360UL*3UL);

  m_thumb_jpeg_buffer_back = std::make_shared<std::vector<uint8_t>>();
  m_thumb_jpeg_buffer_back->reserve(640UL*360UL*3UL);

  m_stopwatch.set_alarm_dt(std::chrono::milliseconds(1000));

  return true;
}

void Thumbnail_sw2_pipe::handle_new_sample()
{
  Glib::RefPtr<Gst::Sample> sample = m_appsink->try_pull_sample(0);
  if(m_stopwatch.is_expired())
  {
    if(sample)
    {
      m_stopwatch.reset();

      Glib::RefPtr<Gst::Buffer> buffer = sample->get_buffer();

      SPDLOG_INFO("Thumbnail_sw2_pipe::handle_new_sample has {}", buffer->get_size());
      {
        std::unique_lock<std::mutex> lock(m_frame_buffer_mutex);

        m_frame_buffer->resize(buffer->get_size());
        uint8_t* out_ptr = m_frame_buffer->data();

        guint num = buffer->n_memory();
        for(guint i = 0; i < num; i++)
        {
          Glib::RefPtr<Gst::Memory> mem_i = buffer->peek_memory(i);

          Gst::MapInfo map_info;
          mem_i->map(map_info, Gst::MAP_READ);

          SPDLOG_INFO("Thumbnail_sw2_pipe::handle_new_sample block {} is {}", i, map_info.get_size());

          guint8* blk_ptr = map_info.get_data();
          gsize   blk_len = map_info.get_size();

          std::copy_n(blk_ptr, blk_len, out_ptr);
          out_ptr += blk_len;

          mem_i->unmap(map_info);
        }
      }

      if( ! downsample_jpeg() )
      {
         SPDLOG_ERROR("Thumbnail_sw2_pipe::downsample_jpeg failed");    
      }
    }
    else
    {
      SPDLOG_WARN("Thumbnail_sw2_pipe::handle_new_sample has null sample"); 
    }
  }
  else
  {
    //not time yet
  }

}

bool Thumbnail_sw2_pipe::downsample_jpeg()
{
  int ret = 0;

  cv::Mat decode_jpeg;
  {  
    std::unique_lock<std::mutex> lock(m_frame_buffer_mutex);

    //decompress full size frame
    cv::Mat rawjpeg(1, m_frame_buffer->size(), CV_8UC1, m_frame_buffer->data());
    decode_jpeg = cv::imdecode(rawjpeg, cv::IMREAD_COLOR);
  }

  //downsample

  //encode
  std::vector< int > params;
  params.push_back(IMWRITE_JPEG_QUALITY);
  params.push_back(75);
  params.push_back(IMWRITE_JPEG_PROGRESSIVE);
  params.push_back(1);
  params.push_back(IMWRITE_JPEG_OPTIMIZE);
  params.push_back(1);
  cv::imencode(".jpg", decode_jpeg, m_thumb_jpeg_buffer_back, params);

  //flip pages
  {  
    std::unique_lock<std::mutex> lock(m_thumb_jpeg_mutex);
    std::swap(m_thumb_jpeg_buffer_front, m_thumb_jpeg_buffer_back);
  }

  return true;
}
