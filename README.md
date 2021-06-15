gst-launch-1.0 nvarguscamerasrc do-timestamp=true num-buffers=10 ! 'video/x-raw(memory:NVMM), width=(int)3280, height=(int)2464' ! nvvidconv ! nvjpegenc ! multifilesink location=test_%05d.jpeg

X1 802.3af / 802.3at POE 30W 37.0-57.0
1 DA+
2 DA-
3 DB+
4 DC+
5 DC-
6 DB-
7 DD+
8 DD-

X2 
1 9V - 36V
2 GND (PWR & SIG)
3 RS232 TX
4 RS232 RX
5 AUX1 / CAN_L
6 AUX2 / CAN_H
7 VDSL2 CPE T1
8 VDSL2 CPE R1

Dependancies
build-essential
gtk-doc-tools
