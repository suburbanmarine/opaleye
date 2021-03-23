
#include "cameras/Logitech_brio.cpp"

#include "http_fcgi_svr.hpp"
#include "http_req_callback_file.hpp"
#include "http_req_jpeg.hpp"
#include "http_req_jsonrpc.hpp"

#include "gst_app.hpp"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <gstreamermm.h>

#include <iostream>

#include <spdlog/spdlog.h>

void start(const std::string& str)
{
	SPDLOG_INFO("start called: {:s}", str);
}

void stop(const std::string& str)
{
	SPDLOG_INFO("stop called: {:s}", str);
}

int main(int argc, char* argv[])
{
	spdlog::set_level(spdlog::level::debug);

	gst_debug_set_default_threshold(GST_LEVEL_INFO);
	// gst_debug_set_default_threshold(GST_LEVEL_TRACE);

	//give gst options
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

  http_fcgi_svr svr;

	std::shared_ptr<http_req_callback_file> req_cb = std::make_shared<http_req_callback_file>();
	svr.register_cb_for_doc_uri("/foo", req_cb);

  svr.start();

	test_app app;
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
	svr.register_cb_for_doc_uri("/cameras/cam0.jpg", jpg_cb);

	std::shared_ptr<jsonrpc::Server> jsonrpc_server = std::make_shared<jsonrpc::Server>();
  jsonrpc::JsonFormatHandler jsonFormatHandler;
  jsonrpc_server->RegisterFormatHandler(jsonFormatHandler);
  jsonrpc_server->GetDispatcher().AddMethod("start", &start);
  jsonrpc_server->GetDispatcher().AddMethod("stop", &stop);
	std::shared_ptr<http_req_jsonrpc> api = std::make_shared<http_req_jsonrpc>();
	api->set_rpc_server(jsonrpc_server);
	svr.register_cb_for_doc_uri("/api/v1", api);

	// Logitech_brio cam;
	// cam.open();
	// cam.start();

	app.make_debug_dot("vid-app");
	app.make_debug_dot_ts("vid-app");

	// app.run();

  // for(size_t i = 0; i < 10; i++)
  for(;;)
  {
    sleep(10);
  }

	// cam.stop();
	// cam.close();

  //stop incoming requests
  svr.stop();

	//stop app
	app.stop();


	return 0;
}
