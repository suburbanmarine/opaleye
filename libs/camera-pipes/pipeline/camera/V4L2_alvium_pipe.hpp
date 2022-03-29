/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#pragma once

#include "V4L2_webcam_pipe.hpp"
#include "v4l2_util.hpp"

class V4L2_alvium_pipe : public V4L2_webcam_pipe
{
public:
  V4L2_alvium_pipe();

  void add_to_bin(const Glib::RefPtr<Gst::Bin>& bin) override;
  bool link_front(const Glib::RefPtr<Gst::Element>& node) override;
  bool link_back(const Glib::RefPtr<Gst::Element>& node) override;

  Glib::RefPtr<Gst::Element> back() override
  {
    return m_out_tee;
  }

  bool init(const char name[]) override;

protected:

};
