FROM ubuntu:20.04

# RUN echo "Acquire::http::Proxy \"http://helios.lan:3142\";" > /etc/apt/apt.conf.d/02proxy


# wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/cuda-ubuntu2004.pin
# mv cuda-ubuntu2004.pin /etc/apt/preferences.d/cuda-repository-pin-600
# apt-key adv --fetch-keys https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/7fa2af80.pub
# sudo add-apt-repository "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2004/x86_64/ /"
# apt-get update
# apt-get -y install cuda

RUN DEBIAN_FRONTEND=noninteractive TZ=Etc/UTC apt-get update && apt-get install -y \ 
	tzdata               \ 
	&& apt-get clean

RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y \ 
	build-essential      \
	cmake                \
	device-tree-compiler \
	gdb                  \
	git                  \
	grunt                \
	gtk-doc-tools        \
	lm-sensors           \
	node-grunt-cli       \
	nodejs               \
	npm                  \
	valgrind             \
	&& apt-get clean


RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y \ 
	googletest \ 
	i2c-tools \ 
	libboost-all-dev \ 
	libfcgi-dev \ 
	libgpiod-dev \ 
	libgrpc++-dev \ 
	libgrpc-dev \ 
	libgstreamer1.0-dev \ 
	libgstreamermm-1.0-dev \ 
	libgstrtspserver-1.0-dev \ 
	libi2c-dev \ 
	liblockfile-dev \ 
	libopencv-contrib-dev \ 
	libopencv-core-dev \ 
	libopencv-imgcodecs-dev \ 
	libopencv-imgproc-dev \ 
	libopencv-video-dev \ 
	libprotobuf-c-dev \ 
	libprotobuf-dev \ 
	libprotoc-dev \ 
	libuvc-dev \ 
	libzmq3-dev \ 
	nvidia-jetpack \ 
	nvidia-l4t-jetson-multimedia-api \ 
	nvidia-l4t-multimedia \ 
	nvidia-l4t-multimedia-utils \ 
	protobuf-c-compiler \ 
	protobuf-compiler \ 
	protobuf-compiler-grpc \ 
	rapidjson-dev \ 
	&& apt-get clean

RUN apt-get update && apt-get install -y \ 
	gstreamer1.0-libav        \
	gstreamer1.0-plugins-bad  \
	gstreamer1.0-plugins-ugly \
#	gstreamer1.0-plugins-rtp  \
	gstreamer1.0-rtsp         \
	libgstreamermm-1.0-dev    \
	libgstrtspserver-1.0-dev  \
	&& apt-get clean

RUN git clone https://github.com/suburbanmarine/opaleye

WORKDIR opaleye

RUN ./generate_cmake_x86.sh