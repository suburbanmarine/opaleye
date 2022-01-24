/**
 * @author Jacob Schloss <jacob.schloss@suburbanmarine.io>
 * @copyright Copyright (c) 2021 Suburban Marine, Inc. All rights reserved.
 * @license Licensed under the 3-Clause BSD LICENSE. See LICENSE.txt for details.
*/

#include "http_fcgi_svr.hpp"
#include "http_req_callback_file.hpp"
#include "http_req_callback_sensors.hpp"
#include "http_req_jpeg.hpp"
#include "http_req_jsonrpc.hpp"
#include "signal_handler.hpp"

#include "zeromq_api_svr.hpp"

#include "Opaleye_app.hpp"
#include "config/Opaleye_config.hpp"

#include "sensor_thread.hpp"
#include "gpio_thread.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <gstreamermm.h>

#include <iostream>
#include <exception>
#include <cstdlib>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "spdlog/async.h"

void terminate_handler()
{
	//flush logs
	spdlog::shutdown();

	//continue dieing
	std::abort();
}

int main(int argc, char* argv[])
{
	//flush logs and clean a little before dieing
	std::set_terminate(&terminate_handler);

	{
		int ret = setenv("GST_DEBUG_DUMP_DOT_DIR", "/tmp", 1);
		if(ret != 0)
		{
			std::cerr << "Could not set GST_DEBUG_DUMP_DOT_DIR" << std::endl;
			return -1;			
		}
	}

	Signal_handler sig_hndl;
	if( ! sig_hndl.mask_all_signals() )
	{
		std::cerr << "Could not mask signals" << std::endl;
		// SPDLOG_ERROR("Could not mask signals");
		return -1;
	}

	gst_debug_set_default_threshold(GST_LEVEL_INFO);
	// gst_debug_set_default_threshold(GST_LEVEL_TRACE);

	//spdlog init
	auto spdlog_glbl_thread_pool = std::make_shared<spdlog::details::thread_pool>(16*1024, 1);
	{
		spdlog::set_level(spdlog::level::debug);
		
		// spdlog::init_thread_pool(16*1024, 1);
		
		std::vector<spdlog::sink_ptr> sinks;
		sinks.push_back( std::make_shared<spdlog::sinks::stdout_color_sink_mt>()             );
		auto logger = std::make_shared<spdlog::async_logger>("log", begin(sinks), end(sinks), spdlog_glbl_thread_pool, spdlog::async_overflow_policy::block);
		logger->set_level(spdlog::level::debug);
		spdlog::set_default_logger( logger );
	}

	//Parse program options
	boost::program_options::variables_map vm;
	{
		//register options
		namespace bpo = boost::program_options;
		bpo::options_description desc("Options"); 
	    desc.add_options() 
			("help"  , "Print usage information and exit")
			("config", bpo::value<std::string>()->default_value("/opt/suburbanmarine/opaleye/conf/config.xml"), "Path to config file")
			;
	
		//Parse options
	    try
	    {
	    	bpo::store(bpo::parse_command_line(argc, argv, desc), vm);

			if(vm.count("help"))
			{
				std::cout << desc << std::endl;
				return 0;
			}

	    	bpo::notify(vm);
	    }
	    catch(const bpo::error& e)
	    {
	    	std::cout << e.what() << std::endl;
		  	std::cout << desc << std::endl;
			return -1;
	    }
	}

	//load config and add a file sink logger
	app_config_mgr cfg_mgr;
	{
		if(!cfg_mgr.deserialize(vm["config"].as<std::string>()))
		{
			std::cout << "cfg_mgr deserialize failed";
			return -1;
		}

		if( ! cfg_mgr.get_config() )
		{
			std::cout << "cfg_mgr does not have a config";
			return -1;	
		}

		{
			boost::filesystem::path log_path = cfg_mgr.get_config()->log_path / "cam-pod_logfile.txt";
			auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_path.string(), 1024*1024, 3, true);
			spdlog::default_logger()->sinks().push_back(file_sink);
		}
	}
	//now spdlog is ready to be used


	//give gst options
	Glib::init();
	Gst::init(argc, argv);

	std::shared_ptr<sensor_thread> sensors = std::make_shared<sensor_thread>();
	if(cfg_mgr.get_config()->sensors_launch == "true")
	{
		SPDLOG_INFO("Init sensors");
		if(!sensors->init())
		{
			SPDLOG_ERROR("sensor thread failed");
			return -1;
		}
		SPDLOG_INFO("Starting sensor thread");
		sensors->launch();
	}

#if 0
	std::shared_ptr<gpio_thread> gpio = std::make_shared<gpio_thread>();
	if(true)
	{
		SPDLOG_INFO("Init gpio");
		if(!gpio->init())
		{
			SPDLOG_ERROR("gpio thread failed");
			return -1;
		}
		SPDLOG_INFO("Starting gpio thread");
		gpio->launch();
	}
#endif

	http_fcgi_svr fcgi_svr;

	std::shared_ptr<http_req_callback_file> req_cb = std::make_shared<http_req_callback_file>();
	fcgi_svr.register_cb_for_doc_uri("/foo", req_cb);

	std::shared_ptr<http_req_callback_sensors> sensor_cb = std::make_shared<http_req_callback_sensors>();
	sensor_cb->init(sensors);
	fcgi_svr.register_cb_for_doc_uri("/api/v1/sensor_types", sensor_cb);
	fcgi_svr.register_cb_for_doc_uri("/api/v1/sensor_types/", sensor_cb);

	SPDLOG_INFO("Starting fcgi connection");
	fcgi_svr.start();

	// test_app_mjpeg app;
	Opaleye_app app;
	app.m_config = cfg_mgr.get_config();
	if( ! app.init() )
	{
		SPDLOG_ERROR("app.init failed");
		return -1;
	}
	if( ! app.start() )
	{
		SPDLOG_ERROR("app.start failed");
		return -1;
	}

	//register http callbacks
	if(app.m_config->camera_configs.count("cam0"))
	{
		std::shared_ptr<http_req_jpeg> jpg_cb = std::make_shared<http_req_jpeg>();
		jpg_cb->set_get_image_cb(std::bind(&Thumbnail_pipe_base::copy_frame_buffer, app.m_pipelines["cam0"]->get_element<Thumbnail_pipe_base>("thumb_0").get(), std::placeholders::_1));
		fcgi_svr.register_cb_for_doc_uri("/cameras/cam0.jpg", jpg_cb);
		fcgi_svr.register_cb_for_doc_uri("/api/v1/cameras/cam0/live/full", jpg_cb);
		fcgi_svr.register_cb_for_doc_uri("/api/v1/cameras/cam0/live/thumb", jpg_cb);
	}

	if(app.m_config->camera_configs.count("cam1"))
	{
		std::shared_ptr<http_req_jpeg> jpg_cb = std::make_shared<http_req_jpeg>();
		jpg_cb->set_get_image_cb(std::bind(&Thumbnail_pipe_base::copy_frame_buffer, app.m_pipelines["cam1"]->get_element<Thumbnail_pipe_base>("thumb_0").get(), std::placeholders::_1));
		fcgi_svr.register_cb_for_doc_uri("/cameras/cam1.jpg", jpg_cb);
		fcgi_svr.register_cb_for_doc_uri("/api/v1/cameras/cam1/live/full", jpg_cb);
		fcgi_svr.register_cb_for_doc_uri("/api/v1/cameras/cam1/live/thumb", jpg_cb);
	}

	if(app.m_config->zeromq_launch == "true")
	{
		SPDLOG_INFO("Starting 0mq svr");
		zeromq_api_svr zmq_svr;	
		zmq_svr.init(app.m_config->zeromq_ep);
		//register 0mq services
		//the camera callbacks are called within the context of a gstreamer thread and should return promptly
		if(app.m_config->camera_configs.count("cam0"))
		{
			std::shared_ptr<nvac_imx219_pipe> cam0 = app.m_pipelines["cam0"]->get_element<nvac_imx219_pipe>("cam_0");
			cam0->set_framebuffer_callback(
				[&zmq_svr](const std::shared_ptr<const std::vector<uint8_t>>& frame_ptr)
				{
					if(frame_ptr)
					{
						zmq_svr.send("/api/v1/cameras/cam0/live/full", "", std::string_view(reinterpret_cast<const char*>(frame_ptr->data()), frame_ptr->size()));
					}
					else
					{
						SPDLOG_ERROR("frame_ptr is null");
					}				
				}
			);
		}

		if(app.m_config->camera_configs.count("cam1"))
		{
			std::shared_ptr<nvac_imx219_pipe> cam1 = app.m_pipelines["cam1"]->get_element<nvac_imx219_pipe>("cam_1");
			cam1->set_framebuffer_callback(
				[&zmq_svr](const std::shared_ptr<const std::vector<uint8_t>>& frame_ptr)
				{
					if(frame_ptr)
					{
						zmq_svr.send("/api/v1/cameras/cam1/live/full", "", std::string_view(reinterpret_cast<const char*>(frame_ptr->data()), frame_ptr->size()));
					}
					else
					{
						SPDLOG_ERROR("frame_ptr is null");
					}
				}
			);
		}
	}

	std::shared_ptr<jsonrpc::Server> jsonrpc_svr_disp = std::make_shared<jsonrpc::Server>();
	jsonrpc::JsonFormatHandler jsonFormatHandler;
	jsonrpc_svr_disp->RegisterFormatHandler(jsonFormatHandler);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_camera",        &Opaleye_app::start_camera,        app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("get_camera_list",     &Opaleye_app::get_camera_list,     app);

	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_still_capture", &Opaleye_app::start_still_capture, app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("stop_still_capture",  &Opaleye_app::stop_still_capture,  app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_video_capture", &Opaleye_app::start_video_capture, app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("stop_video_capture",  &Opaleye_app::stop_video_capture,  app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_rtp_stream",    &Opaleye_app::start_rtp_stream,    app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("stop_rtp_stream",     &Opaleye_app::stop_rtp_stream,     app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("get_pipeline_status", &Opaleye_app::get_pipeline_status, app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("get_pipeline_graph",  &Opaleye_app::get_pipeline_graph,  app);

	jsonrpc_svr_disp->GetDispatcher().AddMethod("set_camera_property", &Opaleye_app::set_camera_property, app);

	std::shared_ptr<http_req_jsonrpc> jsonrpc_api_req = std::make_shared<http_req_jsonrpc>();
	jsonrpc_api_req->set_rpc_server(jsonrpc_svr_disp);
	fcgi_svr.register_cb_for_doc_uri("/api/v1", jsonrpc_api_req);

	// Logitech_brio cam;
	// cam.open();
	// cam.start();

	// app.run();

	std::this_thread::sleep_for(std::chrono::seconds(5));
	if(app.m_config->camera_configs.count("cam0"))
	{
		std::shared_ptr<V4L2_webcam_pipe> m_camera = app.m_pipelines["cam0"]->get_element<V4L2_webcam_pipe>("cam_0");
		if( ! m_camera )
		{
			SPDLOG_ERROR("only V4L2_webcam_pipe camera support now, refactor these to a camera base class");
		}
		else
		{
			m_camera->v4l2_probe();
			m_camera->get_property_description();
		}
	}

	if( ! sig_hndl.mask_def_signals() )
	{
		SPDLOG_ERROR("Could not mask signals");
		return -1;
	}
	while( ! sig_hndl.has_sigint() )
	{
		sig_hndl.wait_for_signal();
	}

	// cam.stop();
	// cam.close();

	//stop incoming requests
	fcgi_svr.stop();

	//stop app
	app.stop();

	if(sensors && sensors->joinable())
	{
		sensors->interrupt();
		sensors->join();
	}

#if 0
	if(gpio && gpio->joinable())
	{
		gpio->interrupt();
		gpio->join();
	}
#endif

	//sync logs - the threadpool dies at end of main so global objects need to stop logging
	spdlog::shutdown();

	return 0;
}
