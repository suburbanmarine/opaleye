

add_executable(alvium_test
	alvium_test.cpp
)

target_link_libraries(alvium_test
	PUBLIC
		opaleye_app
)

add_executable(opaleye_config_test
	opaleye_config_test.cpp
)

target_link_libraries(opaleye_config_test
	PUBLIC
		opaleye_app
)

target_include_directories(opaleye_config_test
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(opaleye_zcm_cli_test
	opaleye_zcm_cli_test.cpp
)

target_link_libraries(opaleye_zcm_cli_test
	PUBLIC
		opaleye_app

		zcm
)

add_executable(opaleye_zeromq_cli_test
	opaleye_zeromq_cli_test.cpp
)

target_link_libraries(opaleye_zeromq_cli_test
	PUBLIC
		opaleye_app

		cppzmq
)

target_include_directories(opaleye_zeromq_cli_test
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(opaleye_zeromq_framegrabber
	opaleye_zeromq_framegrabber.cpp
)

target_link_libraries(opaleye_zeromq_framegrabber
	PUBLIC
		opaleye_app

		cppzmq

        opencv_highgui
        opencv_core
        opencv_imgproc

)

target_include_directories(opaleye_zeromq_framegrabber
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(opaleye_zeromq_svr
	opaleye_zeromq_svr.cpp
)

target_link_libraries(opaleye_zeromq_svr
	PUBLIC
		opaleye_app

		cppzmq
)

target_include_directories(opaleye_zeromq_svr
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(appsrc_test
	appsrc_test.cpp
)

target_link_libraries(appsrc_test
	PUBLIC
		${GST_LIBRARIES}
		Threads::Threads
)

target_include_directories(appsrc_test
	PUBLIC
		${GST_INCLUDE_DIRS}
)

add_executable(appsrc_test_pull
	appsrc_test_pull.cpp
)

target_link_libraries(appsrc_test_pull
	PUBLIC
		${GST_LIBRARIES}
		Threads::Threads
)

target_include_directories(appsrc_test_pull
	PUBLIC
		${GST_INCLUDE_DIRS}
)

add_executable(appsrc_avt_h264_test
	avt_h264_test/appsrc_avt_h264_test.cpp
	avt_h264_test/Pipe_disk.cpp
	avt_h264_test/Pipe_camera.cpp
)

target_link_libraries(appsrc_avt_h264_test
	PUBLIC
		${GST_LIBRARIES}
		Threads::Threads
)

target_include_directories(appsrc_avt_h264_test
	PUBLIC
		${GST_INCLUDE_DIRS}
)

INSTALL(
	TARGETS
		alvium_test
		appsrc_avt_h264_test
		appsrc_test
		opaleye_config_test
	DESTINATION
		bin
)

