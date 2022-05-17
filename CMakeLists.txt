cmake_minimum_required(VERSION 3.10)

project(opaleye 
	VERSION 0.2.0
	DESCRIPTION "Opaleye smart maritime node"
	LANGUAGES C CXX
)

add_custom_command(
	OUTPUT
		${PROJECT_SOURCE_DIR}/external/grpc-web/javascript/net/grpc/web/protoc-gen-grpc-web
	COMMAND
		git submodule update --init && make plugin
	WORKING_DIRECTORY
		${PROJECT_SOURCE_DIR}/external/grpc-web
)

add_custom_target(protoc-gen-grpc-web-target
	DEPENDS
		${PROJECT_SOURCE_DIR}/external/grpc-web/javascript/net/grpc/web/protoc-gen-grpc-web
)

# RAPIDJSON_SSE2
# RAPIDJSON_SSE42
# RAPIDJSON_NEON
add_definitions(
	-DSPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE
	-DBOOST_ALL_DYN_LINK
	-DZMQ_BUILD_DRAFT_API
	# -DMALLOC_CHECK_=2 # debug heap
	# -D_GLIBCXX_DEBUG # changes layout of stl types
)

set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package (Threads REQUIRED)

find_package(Boost
	1.65
	REQUIRED
		filesystem
		program_options
		date_time
		regex
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

find_package( OpenCV REQUIRED )

add_subdirectory(external/spdlog)
add_subdirectory(external/googletest)
# add_subdirectory(external/nvidia-jetpack)


add_library(cppzmq INTERFACE)
target_include_directories(cppzmq
	INTERFACE
		external/cppzmq
)
target_link_libraries(cppzmq
	INTERFACE
		zmq
)

add_library(jsonrpc-lean INTERFACE)
target_include_directories(jsonrpc-lean
	INTERFACE
		external/jsonrpc-lean/include
)

add_subdirectory(libs/camera-pipes)
add_subdirectory(libs/http-bridge)
add_subdirectory(libs/hal)
add_subdirectory(libs/sensors)
add_subdirectory(libs/opaleye-util)
add_subdirectory(opaleye)
add_subdirectory(site)
add_subdirectory(tests)


add_executable(jsontest
	jsontest.cpp
)

# add_custom_command(
# 	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/external/js-cookie
# 
# 	COMMAND npm install -d && grunt
# 
# 	DEPENDS external/js-cookie/src/api.mjs
# 
# 	OUTPUT 
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.js"
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.min.js"
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.min.mjs"
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.mjs"
# 
# 	VERBATIM
# )

# add_custom_target(js-cookie ALL
# 	DEPENDS
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.js"
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.min.js"
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.min.mjs"
# 		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.mjs"
# )

INSTALL(
	FILES
		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.js"
		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.min.js"
		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.min.mjs"
		"${PROJECT_SOURCE_DIR}/external/js-cookie/dist/js.cookie.mjs"
	DESTINATION
		site/static/scripts
)

# add_custom_command(
# 	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/external/jquery
# 	
# 	COMMAND npm run build
# 
# 	DEPENDS external/jquery/src/jquery.js
# 
# 	OUTPUT 
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.map"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.map"
# 
# 	VERBATIM
# )

# add_custom_target(jquery ALL
# 	DEPENDS
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.map"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.js"
# 		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.slim.min.map"
# )

# add_dependencies(vid-enc
# 	jquery
# )

INSTALL(
	FILES
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.js"
		"${PROJECT_SOURCE_DIR}/external/jquery/dist/jquery.min.js"
	DESTINATION
		site/static/scripts
)

INSTALL(
	FILES
		"${PROJECT_SOURCE_DIR}/external/simple-jsonrpc-js/simple-jsonrpc-js.js"
		"${PROJECT_SOURCE_DIR}/external/simple-jsonrpc-js/dist/simple-jsonrpc-js.min.js"
	DESTINATION
		site/static/scripts
)

INSTALL(
	FILES
		"${PROJECT_SOURCE_DIR}/conf/etc/nginx/sites-available/opaleye"
	DESTINATION
		/etc/nginx/sites-available
)

INSTALL(
	FILES
		"${PROJECT_SOURCE_DIR}/conf/etc/samba/smb.conf"
	DESTINATION
		/etc/samba
)

INSTALL(
	FILES
		"${PROJECT_SOURCE_DIR}/conf/etc/udev/rules.d/99-uvc.rules"
	DESTINATION
		/etc/udev/rules.d
)

INSTALL(
	FILES
		"${PROJECT_SOURCE_DIR}/conf/etc/ptp4l.conf"
	DESTINATION
		/etc
)

INSTALL(
	FILES
		scripts/opaleye.service
#		scripts/ptp4l.service
	DESTINATION
		/lib/systemd/system
)

set(CPACK_SET_DESTDIR true)
set(CPACK_INSTALL_PREFIX /opt/suburbanmarine/opaleye)
set(CPACK_GENERATOR "DEB;TGZ")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "SM")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${PROJECT_SOURCE_DIR}/scripts/preinst;${PROJECT_SOURCE_DIR}/scripts/postinst;${PROJECT_SOURCE_DIR}/scripts/prerm;${PROJECT_SOURCE_DIR}/scripts/postrm")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgstreamer1.0-0, libgstreamer-plugins-base1.0-0, libgstreamer-plugins-good1.0-0, libgstreamer-plugins-bad1.0-0, libgstreamermm-1.0-1, libgstrtspserver-1.0-0, libfcgi0ldbl, liblockfile1, graphviz, libfcgi-bin, libi2c0, samba, googletest, v4l-utils, nginx-full")
include(CPack)

