/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http_req_callback_sensors.hpp"
#include "http_req_util.hpp"
#include "http_util.hpp"

#include "linux_thermal_zone.hpp"

#include "Ptree_util.hpp"

#include <Unit_conv.hpp>

#include "http_common.hpp"

#include "path/Path_util.hpp"

#include <boost/filesystem/path.hpp>

#include <boost/regex.hpp>

#include <boost/optional.hpp>

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
    throw MethodNotAllowed("Only GET is accepted");
  }

  boost::regex sensor_uri_regex("^/api/v1/sensor_types(?<hastype>/(?<sensortype>\\w+)(?<hassensors>/sensors(?<hasid>/(?<sensorid>\\d+))?)?)?$");
  
  boost::smatch m;
  bool ret = boost::regex_match(req_util.doc_uri_path.string(), m, sensor_uri_regex);

  if(!ret)
  {
    throw NotFound();
  }

  bool hastype     = m["hastype"].matched;
  bool hassensors  = m["hassensors"].matched;
  bool hassensorid = m["hasid"].matched;

  boost::optional<std::string> sensortype;
  boost::optional<std::string> sensorid;

  if(m["hastype"].matched && m["sensortype"].matched)
  {
    sensortype = std::string(m["sensortype"].first, m["sensortype"].second);
  }
  if(m["hassensors"].matched)
  {
    if(m["hasid"].matched && m["sensorid"].matched)
    {
      sensorid = std::string(m["sensorid"].first, m["sensorid"].second);
    }
  }

  std::string response_str;

  if(hastype && hassensors && hassensorid)
  {
    //specific sensor by type & id
    if( ! sensortype )
    {
      throw BadRequest("sensortype not understood");
    }

    if( ! sensorid )
    {
      throw BadRequest("sensortype not understood");
    }

    if(sensortype.get() == "pressure")
    {
      if(sensorid.get() == "0")
      {
        MS5837_30BA::RESULT baro_data = {};
        m_sensors->get_baro_data(&baro_data);

        boost::property_tree::ptree sample;
        sample.put("value", baro_data.P1_mbar);
        sample.put("unit", "mbar");
        sample.put("timestamp", "0");

        boost::property_tree::ptree sensor_data;
        sensor_data.put_child("sample", sample);

        sensor_data.put("name",     "pressure-0");
        sensor_data.put("location", "external");
        // sensor_data.put_child("min", sensor_min);
        // sensor_data.put_child("max", sensor_max);
        // sensor_data.put_child("precision", sensor_precision);
        // sensor_data.put_child("accuracy", sensor_accuracy);

        response_str = Ptree_util::ptree_to_json_str(sensor_data);
      }
      else
      {
        throw NotFound("sensorid does not exist");
      }
    }
    else if(sensortype.get() == "temperature")
    {
      if(sensorid.get() == "0")
      {
        double ext_temp_data = 0.0;
        m_sensors->get_temp_data(&ext_temp_data);

        boost::property_tree::ptree sample;
        sample.put("value", ext_temp_data);
        sample.put("unit", "degC");
        sample.put("timestamp", "0");

        boost::property_tree::ptree sensor_data;
        sensor_data.put_child("sample", sample);

        sensor_data.put("name",     "temperature-0");
        sensor_data.put("location", "external");
        // sensor_data.put_child("min", sensor_min);
        // sensor_data.put_child("max", sensor_max);
        // sensor_data.put_child("precision", sensor_precision);
        // sensor_data.put_child("accuracy", sensor_accuracy);

        response_str = Ptree_util::ptree_to_json_str(sensor_data);
      }
      else if(sensorid.get() == "1")
      {
        std::map<std::string, double> soc_temp;
        linux_thermal_zone lz;
        if(lz.sample())
        {
          lz.get_temps(&soc_temp);
        }
      }
      else
      {
        throw NotFound("sensorid does not exist");
      }
    }
    else
    {
       throw NotFound("sensortype does not exist");
    }
  }
  else if(hastype && hassensors)
  {
    //list of sensors by type
    if(sensortype.get() == "pressure")
    {
      boost::property_tree::ptree sensor;
      sensor.put("name", "pressure-0");
      sensor.put("id", "0");

      boost::property_tree::ptree sensor_list;
      sensor_list.push_back(std::make_pair("", sensor));

      boost::property_tree::ptree response;
      response.add_child("sensors", sensor_list);

      response_str = Ptree_util::ptree_to_json_str(response);
    }
    else if(sensortype.get() == "temperature")
    {
      //list of sensors
      boost::property_tree::ptree sensor;
      sensor.put("name", "temperature-0");
      sensor.put("id", "0");

      boost::property_tree::ptree sensor_list;
      sensor_list.push_back(std::make_pair("", sensor));

      boost::property_tree::ptree response;
      response.add_child("sensors", sensor_list);

      response_str = Ptree_util::ptree_to_json_str(response);
    }
    else
    {
      throw NotFound();
    }
  }
  else if(hastype)
  {
    //throw 405 method not allowed
    throw MethodNotAllowed();
  }
  else
  {
    //list of types
    boost::property_tree::ptree pressure_type;
    pressure_type.put("type", "pressure");

    boost::property_tree::ptree temperature_type;
    temperature_type.put("type", "temperature");

    boost::property_tree::ptree sensor_types;
    sensor_types.push_back(std::make_pair("", pressure_type));
    sensor_types.push_back(std::make_pair("", temperature_type));

    boost::property_tree::ptree response;
    response.add_child("sensor_types", sensor_types);

    response_str = Ptree_util::ptree_to_json_str(response);
  }

  time_t t_now = time(NULL);
  http_util::HttpDateStr time_str;
  if( ! http_util::time_to_httpdate(t_now, &time_str) )
  {
    throw InternalServerError("Could not get Last-Modified timestamp");
  }

  if(response_str.empty())
  {
    throw ServiceUnavailable();
  }

  FCGX_PutS("Content-Type: application/json\r\n", request->out);
  FCGX_FPrintF(request->out, "Content-Length: %d\r\n", response_str.size());
  FCGX_PutS("Cache-Control: max-age=0, no-store\r\n", request->out);
  FCGX_FPrintF(request->out, "Last-Modified: %s\r\n", time_str.data());
  
  FCGX_PutS("\r\n", request->out);

  FCGX_PutStr(response_str.data(), response_str.size(), request->out);

  #if 0
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
  #endif

  FCGX_Finish_r(request);
}
