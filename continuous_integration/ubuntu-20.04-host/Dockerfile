FROM ubuntu:20.04

# RUN echo "Acquire::http::Proxy \"http://helios10g.lan:3142\";" > /etc/apt/apt.conf.d/02proxy

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
	gdebi \ 
	git   \ 
	rsync \ 
	tzdata \ 
	wget  \ 
	&& apt-get clean

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \ 
	gdb      \ 
	valgrind \ 
	&& apt-get clean

WORKDIR /root
RUN git clone https://github.com/suburbanmarine/opaleye

WORKDIR /root/opaleye
RUN gdebi -n opaleye-*.deb
