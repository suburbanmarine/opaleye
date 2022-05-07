

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

INSTALL(
	TARGETS
		opaleye_config_test
		alvium_test
	DESTINATION
		bin
)

