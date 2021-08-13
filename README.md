# Opaleye, A Smart Maritime Node

## Summary

Opaleye is a demonstration platform for underwater video acquisition and streaming. It has the following primary goals:

 - 4k30 video capture, h.264 encoding, streaming, storage
 - Remote control over embedded web application by human user
 - Remote control over software API, eg json-rpc and gRPC, for system integration
 - Local sensor integration (eg, I2C external temperature and pressure)
 - Health monitoring
 - Operation on 802.3bt POE Gigabit Ethernet tether

The intention is to form a basis for a "smart node" that can be used as part of a larger autonomous or remote-piloted robotic system.

In the future, we hope to grow these capibilities:

 - Edge computing
     - Local reduction and processing of sensor data using DSP, neural networks
 - Networking
     - 1588v2 timestamping - microsecond level accurate global timestamps on video and sensor data across a large network
     - High bandwidth two wire ethernet
 - Video
     - RTSP - automatic video stream negotation
     - RTCP - automatic video stream quality adjustment based on bandwidth

## License

Opaleye is provided under the terms of the BSD-3-Clause license by Suburban Marine, Inc., a California corporation. A copy of this license is in the LICENSE.txt file.

## Contributing

We hope you find Opaleye useful as a reference design of an embedded C++ application. We welcome forks and contributions.

All contributions to Opaleye require a Contributor License Agreement. Please see details at https://code.suburbanmarine.io/cla and the agreement texts in CLA/CLA_entity.md and CLA/CLA_individual.md. Please contact code@suburbanmarine.io with questions or to execute a CLA.

## Hardware Design Files

Portions of the Opaleye reference design are also open hardware.

### BOM

TBD

### Mechanical Drawings

TBD

## Cloning Instructions

Ensure you have ssh keys registered for git-over-ssh based cloning. Opaleye's submodules are referenced via ssh urls and require git-over-ssh.

```console
foo@bar:~$ git clone git@github.com:suburbanmarine/opaleye.git
```

Opaleye uses a number of git submodules, so makre sure those are set up as well. For first clone:

```console
foo@bar:~$ git submodule --init --update
```

And later, to make sure they are checked out at the correct commit:

```console
foo@bar:~$ git submodule --update
```

## Building Instructions

Opaleye provides CMake toolkits for the Jetson nano, Xavier NX, and generic x86. please run the correpsonding generate_cmake_X script so the correct toolchain file is used.

Eg, for the Jetson Nano:

```console
foo@bar:~$ ./generate_cmake_nano.sh
foo@bar:~$ cd build/debug
foo@bar:~$ make -j`nproc`
foo@bar:~$ make package
```

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
 - gdebi
 - graphviz
 - nano
 - nginx-full
 - screen

## API

Opaleye is intended to be controlled by both humans and by other software. An RPC API is provided for software integration.

### HTTP

Opaleye uses the FastCGI protocol to integrate with a web server and respond to client HTTP requests. Normally NGINX is used as the web server, although others that support fcgi will work as well.

### json-rpc over HTTP

Opaleye provides a number of API calls over json-rpc/http at /api/v1.

### gRPC

TBD

### gRPC-Web

TBD

## Supported Hardware

Opaleye is very portable. The main hardware dependency is the hardware accelerated codec pipelines. Support for new hardware platforms mostly consists of adding the few jpeg and h.264 gstreamer blocks and making a new top level application that builds these blocks into the video pipeline. It is expected Opaleye could run with little additional code on NXP i.MX and Qualcomm Snapdragon MPUs.

- Nvidia Xavier NX
- Nvidia Jetson Nano

### Jetson Nano Notes

The Nvidia Jetson Nano must be placed in 10W mode or Max Power mode for 4k video encoding to be sucessful.

5W mode
```console
foo@bar:~$ sudo nvpmodel -m 1
```
10W mode
```console
foo@bar:~$ sudo nvpmodel -m 0
```
Max Power
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

## Standards

 - HTTP
     - rfc2616 - HTTP/1
     - rfc7230 - HTTP/1.1: Message Syntax and Routing
     - rfc7231 - HTTP/1.1: Semantics and Content
     - rfc7234 - HTTP/1.1: Caching 
     - rfc7540 - HTTP/2
 - JSON-RPC 2.0
     - http://www.simple-is-better.org/json-rpc/transport_http.html
     - https://www.simple-is-better.org/json-rpc/jsonrpc20.html
 - Protobuf
 - gRPC

# About

Suburban Marine, Inc. is a nimble engineering company in Southern California.
