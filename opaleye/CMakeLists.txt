add_subdirectory(
	service
)

add_executable(opaleye
	main.cpp
	gst_app.cpp
	gst_filesink_pipeline.cpp
	gst_framesink_pipeline.cpp
	app_config.cpp

	http_req_jpeg.cpp
	http_req_jsonrpc.cpp

	signal_handler.cpp
)

target_link_libraries(opaleye
	PUBLIC
		camera-pipes
		http-bridge

		jsonrpc-lean

		Boost::boost
		Boost::date_time
		Boost::filesystem
		Boost::program_options

		Threads::Threads

		spdlog::spdlog

		#uvc
		fcgi
)

target_include_directories(opaleye
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

INSTALL(
	TARGETS
		opaleye
	DESTINATION
		bin
)

INSTALL(
	FILES
		config.xml
	DESTINATION
		conf
)