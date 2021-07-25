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
		drm
		EGL 
		GLESv2 
		nvbuf_utils
		nvinfer
		nvjpeg
		nvonnxparser
		nvosd
		nvparsers
		v4l2
		X11

		Threads::Threads
)
target_link_directories(
	PUBLIC
		/usr/lib/aarch64-linux-gnu/tegra/
		/usr/local/cuda-10.2/targets/aarch64-linux/lib/
)
