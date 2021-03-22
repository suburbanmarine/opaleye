#include "http_req_jpeg.hpp"
#include "http_util.hpp"

#include <spdlog/spdlog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

void http_req_jpeg::handle(FCGX_Request* const request)
{
  if(m_cam)
  {
    //this is per-req since we could have several threads
    std::shared_ptr<uvc_frame_t> frame_buf;
    m_cam->copy_frame(frame_buf);

    FCGX_PutS("Content-type: image/jpeg\r\n", request->out);
    FCGX_FPrintF(request->out, "Content-length: %d\r\n", frame_buf->data_bytes);
    FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
    // FCGX_PutS("Cache-Control: max-age=0, no-store, must-revalidate\r\n", request->out);
    // FCGX_PutS("X-Accel-Buffering: yes\r\n", request->out);
    
    FCGX_PutS("\r\n", request->out);

    FCGX_PutStr(static_cast<const char*>(frame_buf->data), frame_buf->data_bytes, request->out);
  }
  else
  {
    FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
  }

  FCGX_Finish_r(request);
}
