FROM nvcr.io/nvidia/l4t-base:r32.5.0

RUN echo "Acquire::http::Proxy \"http://helios.lan:3142\";" > /etc/apt/apt.conf.d/02proxy

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
	build-essential      \
	cmake                \
	gdebi                \
	git                  \
	rsync                \
	software-properties-common \
	tzdata               \
	wget                 \
	zip                  \
	&& apt-get clean

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
	default-jdk-headless \
	default-jre-headless \
	device-tree-compiler \
	gdb                  \
	grunt                \
	gtk-doc-tools        \
	lm-sensors           \
	node-grunt-cli       \
	nodejs               \
	npm                  \
	python               \
	valgrind             \
	zip                  \
	&& apt-get clean

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
	googletest             \ 
	i2c-tools              \ 
	libboost-all-dev       \ 
	libelf-dev             \
	libfcgi-dev            \ 
	libgpiod-dev           \ 
	libgrpc++-dev          \ 
	libgrpc-dev            \ 
	libi2c-dev             \ 
	liblockfile-dev        \ 
	libprotobuf-c-dev      \ 
	libprotobuf-dev        \ 
	libprotoc-dev          \ 
	libuvc-dev             \ 
	libzmq3-dev            \ 
	protobuf-c-compiler    \ 
	protobuf-compiler      \ 
	protobuf-compiler-grpc \ 
	rapidjson-dev          \ 
	&& apt-get clean

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
	gstreamer1.0-libav               \ 
	gstreamer1.0-plugins-bad         \ 
	gstreamer1.0-plugins-ugly        \ 
	gstreamer1.0-rtsp                \ 
	libgstreamer1.0-dev              \ 
	libgstreamermm-1.0-dev           \ 
	libgstrtspserver-1.0-dev         \ 
	libopencv-contrib-dev            \ 
	libopencv-core-dev               \ 
	libopencv-dev                    \ 
	libopencv-imgcodecs-dev          \ 
	libopencv-imgproc-dev            \ 
	libopencv-video-dev              \ 
#	gstreamer1.0-plugins-rtp         \
	&& apt-get clean

# RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
# 	nvidia-jetpack                   \ 
# 	nvidia-l4t-jetson-multimedia-api \ 
# 	nvidia-l4t-multimedia            \ 
# 	nvidia-l4t-multimedia-utils      \ 
# 	&& apt-get clean

WORKDIR /root
RUN git clone https://github.com/suburbanmarine/zcm.git
WORKDIR /root/zcm
RUN git checkout patch_installer
RUN ./scripts/make_debian_package.sh
RUN gdebi -n /root/zcm/build/zcm_1.1.5_amd64.deb

WORKDIR /root
RUN git clone https://github.com/suburbanmarine/opaleye

WORKDIR /root/opaleye
RUN git checkout master
RUN git submodule update --init --recursive
RUN ./generate_cmake_xaviernx.sh

WORKDIR /root/opaleye/build/debug
RUN make -j`nproc`
RUN make package