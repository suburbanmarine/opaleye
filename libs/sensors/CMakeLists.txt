add_library(opaleye_sensors
	TSYS01.cpp
	MS5837-30BA.cpp
	linux_thermal_zone.cpp
)

target_link_libraries(opaleye_sensors
	hal
	
	spdlog::spdlog
)

target_include_directories(opaleye_sensors
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
