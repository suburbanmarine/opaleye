add_library(http-bridge
	content_type.cpp
	http_fcgi_svr.cpp
	http_fcgi_work_thread.cpp
	http_req_callback_base.cpp
	http_req_callback_file.cpp
	http_req_util.cpp
	http_util.cpp
	http_common.cpp
	http_req_error_handler.cpp
)

target_link_libraries(http-bridge
	PUBLIC

		opaleye-util

		Boost::boost
		Boost::filesystem
		spdlog::spdlog
)

target_include_directories(http-bridge
	PUBLIC
		include
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_options(http-bridge
	PUBLIC
)

INSTALL(
	TARGETS
		http-bridge
	DESTINATION
		lib
)
