add_library(hal
	i2c_linux.cpp
)

target_link_libraries(hal
	i2c
)
