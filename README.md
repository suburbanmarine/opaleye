# Opaleye Smart Maritime Node

## Summary



## License

Opaleye is provided under the terms of the BSD-3-Clause license by Suburban Marine, Inc., a California corporation. A copy of this license is in the LICENSE.txt file.

## Contributing

We hope you find Opaleye useful as a reference design of an embedded C++ application. 

All contributions to Opaleye require a Contributor License Agreement. Please see details at https://code.suburbanmarine.io/cla and the agreement texts in CLA/CLA_entity.md and CLA/CLA_individual.md. Please contact code@suburbanmarine.io with questions or to execute a CLA.

## Build Dependencies

Opaleye depends on a number of external components.

### Tools
 - build-essential
 - cmake
 - gtk-doc-tools
 - lm-sensors

### Libraries
 - i2c-tools
 - libboost-all-dev
 - libfcgi-dev
 - libgrpc++-dev
 - libgrpc-dev
 - libgstreamer1.0-dev
 - libgstreamermm-1.0-dev
 - libgstrtspserver-1.0-dev
 - liblockfile-dev
 - libprotobuf-c-dev
 - libprotobuf-dev
 - libprotoc-dev
 - libuvc-dev
 - protobuf-c-compiler
 - protobuf-compiler
 - protobuf-compiler-grpc
 - rapidjson-dev
 - nvidia-jetpack
 - nvidia-l4t-jetson-multimedia-api
 - nvidia-l4t-multimedia
 - nvidia-l4t-multimedia-utils

## Runtime Dependencies
 - nano
 - screen
 - gdebi
 - graphviz
 - nginx-full

## API

### HTTP

Opaleye uses the FastCGI protocol to integrate with a web server and respond to client HTTP requests. Opaleye normally uses NGINX as the web server, although others that support fcgi will work as well.

### json-rpc over HTTP

Opaleye provides a number of API calls over json-rpc/http at /api/v1.

### gRPC

TBD

### gRPC-Web

TBD

## Gstreamer notes

gst-launch-1.0 nvarguscamerasrc do-timestamp=true num-buffers=10 ! 'video/x-raw(memory:NVMM), width=(int)3280, height=(int)2464' ! nvvidconv ! nvjpegenc ! multifilesink location=test_%05d.jpeg

## Supported Hardware

Opaleye is very portable, the main hardware dependency is the hardware accelerated codec pipelines. Support for new hardware platforms mostly consists of adding the few jpeg and h.264 gstreamer blocks and making a new top level application that builds these blocks into the video pipeline. It is expected Opaleye could run with little additional code on NXP i.MX and Qualcomm Snapdragon MPUs.

- Nvidia Xavier NX
- Nvidia Jetson nano

### Jetson Notes

5W mode
```console
foo@bar:~$ sudo nvpmodel -m 1
```
10W mode
```console
foo@bar:~$ sudo nvpmodel -m 0
```
MAX POWER
```console
foo@bar:~$ sudo jetson_clocks
```

## Opaleye Pinout

### X1 802.3af / 802.3at POE 30W 37.0-57.0
- 1 DA+
- 2 DA-
- 3 DB+
- 4 DC+
- 5 DC-
- 6 DB-
- 7 DD+
- 8 DD-

### X2
- 1 9V - 36V
- 2 GND (PWR & SIG)
- 3 RS232 TX
- 4 RS232 RX
- 5 AUX1 / CAN_L
- 6 AUX2 / CAN_H
- 7 VDSL2 CPE T1
- 8 VDSL2 CPE R1
