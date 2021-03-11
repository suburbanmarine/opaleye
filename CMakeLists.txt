cmake_minimum_required(VERSION 3.5)
project(vid-enc)

add_definitions(
	-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
	-DBOOST_ALL_DYN_LINK	
)

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

add_subdirectory(camera-pipes)
add_subdirectory(http-bridge)
add_subdirectory(vid-enc)