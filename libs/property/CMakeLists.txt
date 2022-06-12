add_library(property
	Property_base.cpp
	Property.cpp
	Property_enum.cpp
	Property_numeric.cpp
	Property_text.cpp

	Property_factory.cpp

)

target_include_directories(property
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(property
	Boost::boost

	opaleye-util
)
