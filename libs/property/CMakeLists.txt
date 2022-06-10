add_library(property
	Property_base.cpp
	Property.cpp
	Property_enum.cpp
	Property_numeric.cpp
	Property_text.cpp

)

target_include_directories(property
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
)
