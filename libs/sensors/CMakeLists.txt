add_library(sensors
	TSYS01.cpp
	MS5837-30BA.cpp
)

target_link_libraries(sensors
	hal
)

target_include_directories(sensors
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
