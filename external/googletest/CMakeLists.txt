add_library(googletest
	/usr/src/googletest/googletest/src/gtest-all.cc
)

target_include_directories(googletest
	PUBLIC
		/usr/src/googletest/googletest/include
	PRIVATE
		/usr/src/googletest/googletest
)

add_library(googletest_main
	/usr/src/googletest/googletest/src/gtest_main.cc
)

target_link_libraries(googletest_main
	PUBLIC
		googletest
)

add_library(googlemock
	/usr/src/googletest/googlemock/src/gmock-all.cc
)

target_include_directories(googlemock
	PUBLIC
		/usr/src/googletest/googlemock/include
	PRIVATE
		/usr/src/googletest/googlemock
)

target_link_libraries(googlemock
	PUBLIC
		googletest
)

add_library(googlemock_main
	/usr/src/googletest/googlemock/src/gmock_main.cc
)

target_link_libraries(googlemock_main
	PUBLIC
		googlemock
)
