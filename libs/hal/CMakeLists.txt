add_library(hal
	i2c_linux.cpp
)

target_link_libraries(hal
	i2c

	spdlog::spdlog
)

target_include_directories(hal
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
