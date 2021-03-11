add_executable(vid-enc
	main.cpp
	gst_app.cpp
)

target_link_libraries(vid-enc
	camera-pipes
	http-bridge

	Boost::boost
	Boost::date_time
	Boost::filesystem
	Boost::program_options

	uvc
	fcgi
)

target_include_directories(vid-enc
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)