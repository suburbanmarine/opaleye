add_library(jetson_multimedia_api_common SHARED
	/usr/src/jetson_multimedia_api/samples/common/classes/NvApplicationProfiler.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvBuffer.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvDrmRenderer.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvEglRenderer.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvElement.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvElementProfiler.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvJpegDecoder.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvJpegEncoder.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvLogging.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvUtils.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvV4l2Element.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvV4l2ElementPlane.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvVideoConverter.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvVideoDecoder.cpp
	/usr/src/jetson_multimedia_api/samples/common/classes/NvVideoEncoder.cpp
)
target_include_directories(jetson_multimedia_api_common
	PRIVATE
		/usr/src/jetson_multimedia_api/include
		/usr/src/jetson_multimedia_api/include/libjpeg-8b
		/usr/src/jetson_multimedia_api/samples/common/algorithm/cuda
		/usr/src/jetson_multimedia_api/samples/common/algorithm/trt
		/usr/local/cuda/include
		/usr/include/libdrm
		/usr/include/opencv4
)
target_link_libraries(jetson_multimedia_api_common
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
		jetson_multimedia_api_common

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
