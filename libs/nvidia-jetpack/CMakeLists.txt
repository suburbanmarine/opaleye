add_library(nvidia-jetpack INTERFACE)

target_include_directories(nvidia-jetpack
	INTERFACE
		/usr/src/jetson_multimedia_api/include
		/usr/src/jetson_multimedia_api/include/libjpeg-8b
		/usr/local/cuda/include
		/usr/include/libdrm
		/usr/include/opencv4
)

target_link_libraries(nvidia-jetpack
	INTERFACE
		cuda
		cudart
		drm
		EGL 
		GLESv2 
		nvbuf_utils
		nvinfer
		nvjpeg
		nvjpeg
		nvonnxparser
		nvosd
		nvparsers
		v4l2
		X11

		Threads::Threads
)
