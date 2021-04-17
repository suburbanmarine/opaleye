add_library(camera-pipes
	cameras/Logitech_brio.cpp
	cameras/UVC_base.cpp

	pipeline/camera/Logitech_brio_pipe.cpp
	pipeline/camera/Logitech_C525_gst.cpp
	pipeline/camera/Testsrc_pipe.cpp

	pipeline/display/autovideosink_pipe.cpp

	pipeline/decode/jpeg_swdec_pipe.cpp
	pipeline/decode/jpeg_nvdec_pipe.cpp

	pipeline/encode/h264_swenc_pipe.cpp
	pipeline/encode/h264_nvenc_pipe.cpp

	pipeline/storage/mkv_multifilesink_pipe.cpp

	pipeline/stream/rtp_h264_pipe.cpp
	pipeline/stream/udpsink_pipe.cpp
	pipeline/stream/rtpsink_pipe.cpp
	pipeline/stream/rtsp_svr_pipe.cpp

	pipeline/GST_app_base.cpp
	pipeline/GST_element_base.cpp
	pipeline/GST_fakesink.cpp
	pipeline/GST_pipeline_base.cpp
	pipeline/GST_pipeline_base.cpp
	pipeline/mux_pipe.cpp
	pipeline/valve_pipe.cpp
)

target_link_libraries(camera-pipes
	PUBLIC
		Boost::boost

		${GST_LIBRARIES}
		${GSTMM_LIBRARIES}
	#	${GSTRTSP_LIBRARIES}

		spdlog::spdlog
)

target_include_directories(camera-pipes
	PUBLIC
		include
		${CMAKE_CURRENT_SOURCE_DIR}
		${GST_INCLUDE_DIRS}
		${GSTMM_INCLUDE_DIRS}
#		${GSTRTSP_INCLUDE_DIRS}
)

target_compile_options(camera-pipes
	PUBLIC
		${GST_CFLAGS}
		${GSTMM_CFLAGS}
#		${GSTRTSP_CFLAGS}
)

INSTALL(
	TARGETS
		camera-pipes
	DESTINATION
		lib
)
