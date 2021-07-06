
#include "cameras/Logitech_brio.cpp"

#include "http_fcgi_svr.hpp"
#include "http_req_callback_file.hpp"
#include "http_req_jpeg.hpp"
#include "http_req_jsonrpc.hpp"
#include "signal_handler.hpp"

#include "gst_app.hpp"
#include "app_config.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <gstreamermm.h>

#include <iostream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include "spdlog/async.h"

int main(int argc, char* argv[])
{
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

	spdlog::set_level(spdlog::level::debug);

	spdlog::init_thread_pool(1024, 1);

	std::vector<spdlog::sink_ptr> sinks;
	sinks.push_back( std::make_shared<spdlog::sinks::rotating_file_sink_mt>("/opt/suburbanmarine/opaleye/log/cam-pod_logfile.txt", 1024*1024, 3, true) );
	sinks.push_back( std::make_shared<spdlog::sinks::stdout_color_sink_mt>()             );
	auto tp2 = std::make_shared<spdlog::details::thread_pool>(1024, 1);
	auto logger = std::make_shared<spdlog::async_logger>("log", begin(sinks), end(sinks), tp2, spdlog::async_overflow_policy::block);
	spdlog::set_default_logger( logger );

	//give gst options
	Glib::init();
	Gst::init(argc, argv);

	//Parse program options
	boost::program_options::variables_map vm;
	{
		//register options
		namespace bpo = boost::program_options;
		bpo::options_description desc("Options"); 
	    desc.add_options() 
			("help"  , "Print usage information and exit")
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

	app_config_mgr cfg_mgr;
	if(!cfg_mgr.deserialize("/opt/suburbanmarine/opaleye/conf/config.xml"))
	{
		SPDLOG_ERROR("cfg_mgr deserialize failed");
		return -1;
	}

	if( ! cfg_mgr.get_config() )
	{
		SPDLOG_ERROR("cfg_mgr does not have a config");
		return -1;	
	}

	http_fcgi_svr fcgi_svr;

	std::shared_ptr<http_req_callback_file> req_cb = std::make_shared<http_req_callback_file>();
	fcgi_svr.register_cb_for_doc_uri("/foo", req_cb);

	fcgi_svr.start();

	test_app app;
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

	std::shared_ptr<http_req_jpeg> jpg_cb = std::make_shared<http_req_jpeg>();
	jpg_cb->set_cam(&app.m_logi_brio);
	fcgi_svr.register_cb_for_doc_uri("/cameras/cam0.jpg", jpg_cb);

	std::shared_ptr<jsonrpc::Server> jsonrpc_svr_disp = std::make_shared<jsonrpc::Server>();
	jsonrpc::JsonFormatHandler jsonFormatHandler;
	jsonrpc_svr_disp->RegisterFormatHandler(jsonFormatHandler);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_camera", &test_app::start_camera, app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("get_camera_list", &test_app::get_camera_list, app);

	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_still_capture", &test_app::start_still_capture, app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("stop_still_capture",  &test_app::stop_still_capture,  app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_video_capture", &test_app::start_video_capture, app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("stop_video_capture",  &test_app::stop_video_capture,  app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("start_rtp_stream",    &test_app::start_rtp_stream,    app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("stop_rtp_stream",     &test_app::stop_rtp_stream,     app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("get_pipeline_status", &test_app::get_pipeline_status, app);
	jsonrpc_svr_disp->GetDispatcher().AddMethod("get_pipeline_graph",  &test_app::get_pipeline_graph,  app);

	std::shared_ptr<http_req_jsonrpc> jsonrpc_api_req = std::make_shared<http_req_jsonrpc>();
	jsonrpc_api_req->set_rpc_server(jsonrpc_svr_disp);
	fcgi_svr.register_cb_for_doc_uri("/api/v1", jsonrpc_api_req);

	// Logitech_brio cam;
	// cam.open();
	// cam.start();

	app.make_debug_dot("vid-app");
	app.make_debug_dot_ts("vid-app");

	// app.run();

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

	//sync logs
	spdlog::shutdown();

	return 0;
}