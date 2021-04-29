FROM nvcr.io/nvidia/l4t-base:r32.4.4

RUN echo "Acquire::http::Proxy \"http://helios.lan:3142\";" > /etc/apt/apt.conf.d/02proxy

RUN apt-get update && apt-get install -y \ 
	build-essential \
	cmake           \
	gdb             \
	git             \
	valgrind        \
	&& apt-get clean

RUN apt-get update && apt-get install -y \ 
	libboost-all-dev          \
	liblockfile-dev           \
	libfcgi-dev               \
	rapidjson-dev             \
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
