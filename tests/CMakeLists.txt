

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

INSTALL(
	TARGETS
		opaleye_config_test
		alvium_test
	DESTINATION
		bin
)

