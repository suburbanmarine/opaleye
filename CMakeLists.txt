cmake_minimum_required(VERSION 3.5)
project(vid-enc)

add_definitions(
	-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
	-DBOOST_ALL_DYN_LINK	
)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

find_package(Boost
	1.65
	REQUIRED
		filesystem
		program_options
		date_time
)

find_package(PkgConfig)

pkg_check_modules(GST
	REQUIRED
		gstreamer-1.0
	  gstreamer-app-1.0
	  gstreamer-base-1.0
	  gstreamer-sdp-1.0
	  gstreamer-video-1.0
)

pkg_check_modules(GSTMM
	REQUIRED
		gstreamermm-1.0
)

pkg_check_modules(GSTRTSP
	REQUIRED
		gstreamer-rtsp-1.0
)

add_subdirectory(external/spdlog)

add_library(jsonrpc-lean INTERFACE)
target_include_directories(jsonrpc-lean
	INTERFACE
		external/jsonrpc-lean/include
)

add_subdirectory(camera-pipes)
add_subdirectory(http-bridge)
add_subdirectory(vid-enc)

add_custom_command(
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/external/jquery
	
	COMMAND npm run build

	DEPENDS external/jquery/src/jquery.js

	OUTPUT 
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.map"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.map"

	VERBATIM
)

add_custom_target(jquery
	DEPENDS
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.map"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.map"
)

add_dependencies(vid-enc
	jquery
)

#INSTALL(
#	TARGETS
#		jquery
#	DESTINATION
#		var/www
#)

include(CPack)