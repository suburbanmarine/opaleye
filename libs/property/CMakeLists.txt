add_library(property
	Property_base.cpp
	Property.cpp
)

target_include_directories(property
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
