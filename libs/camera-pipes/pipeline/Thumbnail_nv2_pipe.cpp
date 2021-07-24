#include "Thumbnail_nv2_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include </usr/src/jetson_multimedia_api/include/NvJpegDecoder.h>
#include </usr/src/jetson_multimedia_api/include/NvJpegEncoder.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

Thumbnail_nv2_pipe::Thumbnail_nv2_pipe()
{
  
}

void Thumbnail_nv2_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}

bool Thumbnail_nv2_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  return false;
}

bool Thumbnail_nv2_pipe::init(const char name[])
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
      "pixel-aspect-ratio", Gst::Fraction(1, 1),
      // "format","JPG",
      "width",              3840,
      "height",             2160
      );

    m_appsink = Gst::AppSink::create();
    m_appsink->property_caps().set_value(m_appsink_caps);
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

  m_jpegenc = std::shared_ptr<NvJPEGEncoder>(NvJPEGDecoder:createJPEGEncoder("jpegenc"));
  m_jpegdec = std::shared_ptr<NvJPEGDecoder>(NvJPEGEncoder::createJPEGDecoder("jpegdec"));

  return true;
}

void Thumbnail_nv2_pipe::handle_new_sample()
{
  Glib::RefPtr<Gst::Sample> sample = m_appsink->try_pull_sample(0);
  if(sample && m_stopwatch.is_expired())
  {
    m_stopwatch.reset();

    Glib::RefPtr<Gst::Buffer> buffer = sample->get_buffer();

    SPDLOG_INFO("Thumbnail_nv2_pipe::handle_new_sample has {}", buffer->get_size());
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

        SPDLOG_INFO("Thumbnail_nv2_pipe::handle_new_sample block {} is {}", i, map_info.get_size());

        guint8* blk_ptr = map_info.get_data();
        gsize   blk_len = map_info.get_size();

        std::copy_n(blk_ptr, blk_len, out_ptr);
        out_ptr += blk_len;

        mem_i->unmap(map_info);
      }
    }
  }
  else
  {
    SPDLOG_INFO("Thumbnail_nv2_pipe::handle_new_sample has null sample"); 
  }

  downsample_jpeg();
}


#include "NvUtils.h"
#include "NvCudaProc.h"
#include "nvbuf_utils.h"
#include "NvVideoEncoder.h"
#include "NvVideoDecoder.h"


bool downsample_jpeg()
{
  int pixfmt = 0;
  int width  = 0;
  int height = 0;
  {  
    std::unique_lock<std::mutex> lock(m_frame_buffer_mutex);

    //decompress full size frame
    NvBuffer* frame_buffer = nullptr;
    m_jpegdec->decodeToBuffer(&frame_buffer, m_frame_buffer.data(), m_frame_buffer.size(), pixfmt, width, height)
  }


  //downsample
  NvBufferTransformParams transform_params;
  memset(&transform_params, 0, sizeof(transform_params));

  /** flag to indicate which of the transform parameters are valid. */
  transform_params.transform_flag   = NVBUFFER_TRANSFORM_FILTER;
  transform_params.transform_flip   = NvBufferTransform_None;
  transform_params.transform_filter = NvBufferTransform_Filter_Bilinear;
  transform_params.src_rect.top;
  transform_params.src_rect.left;
  transform_params.src_rect.width;
  transform_params.src_rect.height;

  transform_params.dst_rect.top;
  transform_params.dst_rect.left;
  transform_params.dst_rect.width;
  transform_params.dst_rect.height;
  transform_params.transform_params.session = NULL;

  int thumb_width  = width  / 6;
  int thumb_height = height / 6;

  NvBuffer thumb_buffer(pixfmt, thumb_width, thumb_height, 0);
  buffer.allocateMem();

  int ret = NvBufferTransform(frame_buffer->planes[0].fd, thumb_buffer.planes[0].fd, &transform_params)

  //compress thumbnail
  NvBuffer jpeg_thumb_buffer(V4L2_PIX_FMT_YUV420M, thumb_width, thumb_height, 0);
  buffer.allocateMem();

  m_thumb_jpeg_buffer_back.resize(m_thumb_jpeg_buffer_back.capacity());
  unsigned long out_buf_size = m_thumb_jpeg_buffer_back.capacity()*sizeof(m_thumb_jpeg_buffer_back[0]);
  m_jpegenc->encodeFromBuffer(thumb_buffer, JCS_YCbCr, m_thumb_jpeg_buffer_back.data(), out_buf_size, 75), 
  m_thumb_jpeg_buffer_back.resize(out_buf_size);

  //flip pages
  {  
    std::unique_lock<std::mutex> lock(m_thumb_jpeg_mutex);
    m_thumb_jpeg_buffer_front.swap(m_thumb_jpeg_buffer_back);
  }

  frame_buffer->deallocateMemory();
  thumb_buffer.deallocateMemory();
  jpeg_thumb_buffer.deallocateMemory();

}
