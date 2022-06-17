add_library(property
	Property_exceptions.cpp

	Property_base.cpp
	Property.cpp
	Property_enum.cpp
	Property_numeric.cpp
	Property_text.cpp

	Property_ipv4_endpoint.cpp
	Property_ipv6_endpoint.cpp
	Property_ipv4_network.cpp
	Property_ipv6_network.cpp

	Property_factory.cpp

)

target_include_directories(property
	PUBLIC
		include
)

target_link_libraries(property
	Boost::boost

	opaleye-util
)

add_executable(property-tests
	tests/Property_numeric_tests.cpp
)

target_link_libraries(property-tests
	property
	googletest
	googletest_main
)

target_include_directories(property-tests
	PUBLIC
		
)
