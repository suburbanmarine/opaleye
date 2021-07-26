add_library(nvidia-jetpack INTERFACE)

target_include_directories(nvidia-jetpack
	INTERFACE
		/usr/src/jetson_multimedia_api/include
		/usr/src/jetson_multimedia_api/include/libjpeg-8b
		/usr/src/jetson_multimedia_api/samples/common/algorithm/cuda
		/usr/src/jetson_multimedia_api/samples/common/algorithm/trt
		/usr/local/cuda/include
		/usr/include/libdrm
		/usr/include/opencv4
)

target_link_libraries(nvidia-jetpack
	INTERFACE
		cuda
		cudart
		nvbuf_utils
		nvinfer
		nvjpeg
		nvonnxparser
		nvosd
		nvparsers
		
		drm
		EGL 
		GLESv2 
		v4l2
		X11

		Threads::Threads
)
target_link_directories(nvidia-jetpack
	INTERFACE
		/usr/lib/aarch64-linux-gnu/tegra/
		/usr/local/cuda-10.2/targets/aarch64-linux/lib/
)
