
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

add_executable(opaleye_zeromq
	opaleye_zeromq.cpp
)

target_link_libraries(opaleye_zeromq
	PUBLIC
		zmq
)

target_include_directories(opaleye_zeromq
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

INSTALL(
	TARGETS
		opaleye_config_test
	DESTINATION
		bin
)

