add_executable(vid-enc
	main.cpp
	gst_app.cpp
)

target_link_libraries(vid-enc
	PUBLIC
		camera-pipes
		http-bridge

		Boost::boost
		Boost::date_time
		Boost::filesystem
		Boost::program_options

		Threads::Threads

		spdlog::spdlog

		uvc
		fcgi
)

target_include_directories(vid-enc
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)