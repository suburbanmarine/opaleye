add_library(opaleye-util
	errno_util.cpp
	Unit_conv.cpp
	Stopwatch.cpp
)

target_link_libraries(opaleye-util
	rt
)

target_include_directories(opaleye-util
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
