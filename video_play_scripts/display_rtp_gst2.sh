gst-launch-1.0 udpsrc port=5002 caps="application/x-rtp" ! rtpjitterbuffer drop-on-latency=true latency=50 ! rtph264depay ! decodebin ! autovideosink
