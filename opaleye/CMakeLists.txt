add_subdirectory(
	service
)

add_library(opaleye_app
	Opaleye_app.cpp
	gst_app_mjpeg.cpp
	gst_filesink_pipeline.cpp
	gst_framesink_pipeline.cpp
	
	config/Opaleye_config.cpp
	config/Opaleye_config_camera.cpp
	config/Opaleye_config_pipeline.cpp
	config/Opaleye_config_mgr.cpp

	http_req_jpeg.cpp
	http_req_jsonrpc.cpp
	http_req_callback_sensors.cpp

	sensor_thread.cpp
	# gpio_thread.cpp

	signal_handler.cpp

	zeromq_api.cpp
)

target_include_directories(opaleye_app
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(opaleye_app
	PUBLIC
		camera-pipes
		camera-pipes-nv
		http-bridge
		opaleye_sensors

		jsonrpc-lean

		Boost::boost
		Boost::date_time
		Boost::filesystem
		Boost::program_options
		Boost::regex

		Threads::Threads

		spdlog::spdlog

		#uvc
		fcgi

		zmq

		rt
)

add_executable(opaleye
	main.cpp
)

target_link_libraries(opaleye
	PUBLIC
		opaleye_app
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
	TARGETS
		opaleye_app
	DESTINATION
		lib
)

INSTALL(
	FILES
		config.xml
	DESTINATION
		conf
)