#include "Thumbnail_nv2_pipe.hpp"

#include "pipeline/gst_common.hpp"

#include <gstreamermm/buffer.h>
#include <gstreamermm/elementfactory.h>

#include <spdlog/fmt/ostr.h>
#include <spdlog/fmt/fmt.h>

Thumbnail_nv2_pipe::Thumbnail_nv2_pipe()
{
  m_thumb_jpeg_buffer_front      = nullptr;
  m_thumb_jpeg_buffer_front_size = 0;
  m_thumb_jpeg_buffer_back       = nullptr;
  m_thumb_jpeg_buffer_back_size  = 0;  
}
Thumbnail_nv2_pipe::~Thumbnail_nv2_pipe()
{
  if(m_thumb_jpeg_buffer_front)
  {
    free(m_thumb_jpeg_buffer_front);
    m_thumb_jpeg_buffer_front = nullptr; 
  }
  if(m_thumb_jpeg_buffer_back)
  {
    free(m_thumb_jpeg_buffer_back);
    m_thumb_jpeg_buffer_back = nullptr; 
  }
}

void Thumbnail_nv2_pipe::add_to_bin(const Glib::RefPtr<Gst::Bin>& bin)
{
  bin->add(m_bin);
}

bool Thumbnail_nv2_pipe::link_front(const Glib::RefPtr<Gst::Element>& node)
{
  node->link(m_in_queue);
  return true;
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

  m_jpegenc = NvJPEGEncoder::createJPEGEncoder("jpegenc");
  m_jpegdec = NvJPEGDecoder::createJPEGDecoder("jpegdec");

  m_stopwatch.set_alarm_dt(std::chrono::milliseconds(1000));

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

    if( ! downsample_jpeg() )
    {
       SPDLOG_ERROR("Thumbnail_nv2_pipe::downsample_jpeg failed");    
    }
  }
  else
  {
    SPDLOG_INFO("Thumbnail_nv2_pipe::handle_new_sample has null sample"); 
  }

}

bool Thumbnail_nv2_pipe::downsample_jpeg()
{
  int ret = 0;

  nv_dma_buf frame_buffer_fd;
  uint32_t pixfmt = 0;
  uint32_t width  = 0;
  uint32_t height = 0;
  {  
    std::unique_lock<std::mutex> lock(m_frame_buffer_mutex);

    //decompress full size frame
    int temp_fd = -1;
    ret = m_jpegdec->decodeToFd(temp_fd, m_frame_buffer->data(), m_frame_buffer->size(), pixfmt, width, height);
    if(ret != 0)
    {
      SPDLOG_ERROR("m_jpegdec->decodeToBuffer failed");
      return false;
    }

    frame_buffer_fd.m_fd = temp_fd;
  }

  NvBufferParamsEx frame_buffer_params;
  memset(&frame_buffer_params, 0, sizeof(frame_buffer_params));
  ret = NvBufferGetParamsEx(frame_buffer_fd.m_fd, &frame_buffer_params);
  if(ret != 0)
  {
    SPDLOG_ERROR("NvBufferGetParamsEx failed");
    return false;
  }

  //downsample
  const int thumb_width  = width  / 6;
  const int thumb_height = height / 6;

  NvBufferTransformParams transform_params;
  memset(&transform_params, 0, sizeof(transform_params));
  transform_params.transform_flag   = NVBUFFER_TRANSFORM_FILTER;
  transform_params.transform_flip   = NvBufferTransform_None;
  transform_params.transform_filter = NvBufferTransform_Filter_Bilinear; // NvBufferTransform_Filter_Nearest, NvBufferTransform_Filter_Smart, NvBufferTransform_Filter_Nicest
  transform_params.src_rect.top     = 0;
  transform_params.src_rect.left    = 0;
  transform_params.src_rect.width   = width;
  transform_params.src_rect.height  = height;
  transform_params.dst_rect.top     = 0;
  transform_params.dst_rect.left    = 0;
  transform_params.dst_rect.width   = thumb_width;
  transform_params.dst_rect.height  = thumb_height;
  transform_params.session = NULL;

  //todo - input size changes rarely, cache this thumb buffer so we don't need to alloc all the time
  nv_dma_buf thumb_buffer_fd;
  NvBufferCreateParams thumb_buffer_params;
  memset(&thumb_buffer_params, 0, sizeof(thumb_buffer_params));
  thumb_buffer_params.width       = thumb_width;
  thumb_buffer_params.height      = thumb_height;
  thumb_buffer_params.layout      = NvBufferLayout_Pitch;
  thumb_buffer_params.payloadType = NvBufferPayload_SurfArray;
  thumb_buffer_params.colorFormat = frame_buffer_params.params.pixel_format;
  thumb_buffer_params.nvbuf_tag   = NvBufferTag_NONE;
  {
    int temp_fd = -1;
    ret = NvBufferCreateEx(&temp_fd, &thumb_buffer_params);
    if(ret != 0)
    {
      SPDLOG_ERROR("NvBufferCreateEx failed");
      return false;
    }

    thumb_buffer_fd.m_fd = temp_fd;
  }

  ret = NvBufferTransform(frame_buffer_fd.m_fd, thumb_buffer_fd.m_fd, &transform_params);
  if(ret != 0)
  {
    SPDLOG_ERROR("NvBufferTransform failed");
    return false;
  }

  // m_jpegenc->setScaledEncodeParams(6, 6);// does this scale be 1/6? That could be really nice
  m_jpegenc->encodeFromFd(thumb_buffer_fd.m_fd, JCS_YCbCr, &m_thumb_jpeg_buffer_back, m_thumb_jpeg_buffer_back_size, 75);
  if(ret != 0)
  {
    SPDLOG_ERROR("jpeg_thumb_buffer.encodeFromBuffer failed");
    return false;
  }

  //flip pages
  {  
    std::unique_lock<std::mutex> lock(m_thumb_jpeg_mutex);
    std::swap(m_thumb_jpeg_buffer_front, m_thumb_jpeg_buffer_back);
    std::swap(m_thumb_jpeg_buffer_front_size, m_thumb_jpeg_buffer_back_size);
  }

  return true;
}
