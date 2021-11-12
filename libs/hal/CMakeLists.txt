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

function(gen_dtb BASENAME)

	set(SOURCE_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${BASENAME}.dts)
	set(OUTPUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/${BASENAME}.dtb)

	add_custom_command(
		DEPENDS ${SOURCE_FILE}
		OUTPUT  ${OUTPUT_FILE}

		COMMAND dtc -I dts -O dtb -o ${OUTPUT_FILE} ${SOURCE_FILE}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
		VERBATIM
	)


endfunction()