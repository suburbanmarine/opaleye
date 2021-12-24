/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http_req_callback_sensors.hpp"
#include "http_req_util.hpp"
#include "http_util.hpp"

#include "linux_thermal_zone.hpp"

#include <Unit_conv.hpp>

#include "http_common.hpp"

#include "path/Path_util.hpp"

#include <boost/filesystem/path.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/printf.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

#include <sstream>

void http_req_callback_sensors::handle(FCGX_Request* const request)
{
  http_req_util req_util;
  req_util.load(request);
  req_util.log_request_env();

  if(req_util.request_method_enum != http_common::REQUEST_METHOD::GET)
  {
    throw BadRequest("Only GET is accepted");
  }

  boost::filesystem::path base_uri        = "/api/v1/sensors";
  boost::filesystem::path temperature_uri = "/api/v1/sensors/temperature";
  boost::filesystem::path pressure_uri    = "/api/v1/sensors/pressure";


  boost::filesystem::path norm_doc_uri_path;
  if(Path_util::trailing_element_is_dir(req_util.doc_uri_path))
  {
    norm_doc_uri_path = req_util.doc_uri_path.parent_path();
    SPDLOG_WARN("Converting {} to {}", req_util.doc_uri_path, norm_doc_uri_path);
  }
  else
  {
    norm_doc_uri_path = req_util.doc_uri_path;
  }

  if(Path_util::is_parent_path(temperature_uri, req_util.doc_uri_path))
  {

  }
  else if(Path_util::is_parent_path(pressure_uri, req_util.doc_uri_path))
  {

  }
  else if(Path_util::is_parent_path(base_uri, req_util.doc_uri_path))
  {
    // handle_index();
  }
  else
  {

  }

  //this is per-req since we could have several threads
  double ext_temp_data = 0.0;
  m_sensors->get_temp_data(&ext_temp_data);

  MS5837_30BA::RESULT baro_data = {};
  m_sensors->get_baro_data(&baro_data);

  std::map<std::string, double> soc_temp;
  linux_thermal_zone lz;
  if(lz.sample())
  {
    lz.get_temps(&soc_temp);
  }

  time_t t_now = time(NULL);
  http_util::HttpDateStr time_str;
  if( ! http_util::time_to_httpdate(t_now, &time_str) )
  {
    throw InternalServerError("Could not get Last-Modified timestamp");
  }

  if(true)
  {
    if(true)
    {
      boost::property_tree::ptree external_sensor_data;
      external_sensor_data.put<double>("pressure",    baro_data.P1_mbar);
      external_sensor_data.put<std::string>("pressure.<xmlattr>.unit", "mbar");
      external_sensor_data.put<double>("temperature", ext_temp_data);
      external_sensor_data.put<std::string>("pressure.<xmlattr>.unit", "degC");

      boost::property_tree::ptree internal_sensor_data;
      for(const auto t : soc_temp)
      {
        internal_sensor_data.put<double>(t.first, t.second);
        internal_sensor_data.put<std::string>(t.first + ".<xmlattr>.unit", "degC");
      }

      boost::property_tree::ptree temp;
      temp.put_child("sensors.external", external_sensor_data);
      temp.put_child("sensors.internal", internal_sensor_data);

      std::stringstream ss;
      boost::property_tree::write_xml(ss, temp);
      SPDLOG_DEBUG("sensor: {:s}", ss.str());

      std::string str = ss.str();

      FCGX_PutS("Content-Type: text/xml\r\n", request->out);
      // FCGX_PutS("Content-Type: text/html\r\n", request->out);
      FCGX_FPrintF(request->out, "Content-Length: %d\r\n", str.size());
      FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
      FCGX_FPrintF(request->out, "Last-Modified: %s\r\n", time_str.data());
      
      FCGX_PutS("\r\n", request->out);

      FCGX_PutStr(str.data(), str.size(), request->out);
    }
    else
    {
      std::stringstream ss;

      ss << fmt::format("External\r\n");
      ss << fmt::format("\tTemp: {:0.1f} degC / {:0.1f} degF\r\n", ext_temp_data, Unit_conv::degC_to_degF(ext_temp_data));
      ss << fmt::format("\tPres: {:d} mbar\r\n", baro_data.P1_mbar);
      ss << fmt::format("\r\n");
      ss << fmt::format("Internal\r\n");
      for(const auto t : soc_temp)
      {
        ss << fmt::format("\t{:s}: {:0.3f} degC\r\n", t.first, t.second);
      }

      std::string str = ss.str();

      FCGX_PutS("Content-Type: text/plain\r\n", request->out);
      // FCGX_PutS("Content-Type: text/html\r\n", request->out);
      FCGX_FPrintF(request->out, "Content-Length: %d\r\n", str.size());
      FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
      FCGX_FPrintF(request->out, "Last-Modified: %s\r\n", time_str.data());
      
      FCGX_PutS("\r\n", request->out);

      FCGX_PutStr(str.data(), str.size(), request->out);
    }
  }
  else
  {
    FCGX_PutS("Status: 500 Internal Error\r\n", request->out);
  }

  FCGX_Finish_r(request);
}
