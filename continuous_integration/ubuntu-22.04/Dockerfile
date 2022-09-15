FROM ubuntu:22.04

# RUN echo "Acquire::http::Proxy \"http://helios.lan:3142\";" > /etc/apt/apt.conf.d/02proxy

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

# WORKDIR /tmp
# RUN wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.0-1_all.deb && \
#	dpkg -i cuda-keyring_1.0-1_all.deb && \
#	rm cuda-keyring_1.0-1_all.deb
# RUN add-apt-repository "deb https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/ /"

# RUN echo "deb https://repo.download.nvidia.com/jetson/common r32.2 main" >> /etc/apt/sources.list.d/jetson-multimedia.repo
# RUN echo "deb https://repo.download.nvidia.com/jetson/t194 r32.2 main" >> /etc/apt/sources.list.d/jetson-multimedia.list


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
	python-is-python3    \
	python3              \
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
RUN ./generate_cmake_x86.sh

WORKDIR /root/opaleye/build/debug
RUN make -j`nproc`
RUN make package

WORKDIR /root/opaleye/build/release
RUN make -j`nproc`
RUN make package