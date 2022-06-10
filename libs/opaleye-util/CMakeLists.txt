add_library(opaleye-util
	chrono_util.cpp
	errno_util.cpp
	Ptree_util.cpp
	Stopwatch.cpp
	thread_base.cpp
	Unit_conv.cpp

	path/Directory_tree.cpp
	path/Directory_tree_node.cpp
	path/Path_util.cpp
)

target_link_libraries(opaleye-util
	Boost::boost
	Boost::filesystem
	rt
	spdlog
)

target_include_directories(opaleye-util
	PUBLIC
		include
)

add_executable(opaleye-util-tests
	tests/path/Directory_tree_tests.cpp
	tests/path/Directory_tree_node_tests.cpp
	tests/path/Path_util_tests.cpp
)

target_link_libraries(opaleye-util-tests
	opaleye-util
	googletest
	googletest_main
)

target_include_directories(opaleye-util-tests
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
