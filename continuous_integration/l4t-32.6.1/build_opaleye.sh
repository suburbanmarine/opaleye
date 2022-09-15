#!/bin/bash

trap 'exit -1' err

# docker buildx create --name multibuilder --use
# docker buildx build --platform linux/arm64 --output=type=registry -t suburbanmarine/sculpin_mavproxy:latest mavproxy_ubu20.04
docker buildx build --platform linux/arm64 --output=type=registry -t suburbanmarine/opaleye:l4t-32.6.1 .

# CONTAINER_ID=$(docker create -it sculpin_mavproxy:latest /bin/bash)

# docker start $CONTAINER_ID

# docker exec -w /root/mavlink $CONTAINER_ID git checkout 1.0.12
# docker exec -w /root/mavlink $CONTAINER_ID git submodule sync
# docker exec -w /root/mavlink $CONTAINER_ID git submodule update --init --recursive

# docker exec -w /root/pymavlink $CONTAINER_ID git checkout 2.4.30
# docker exec -w /root/pymavlink $CONTAINER_ID git submodule sync
# docker exec -w /root/pymavlink $CONTAINER_ID git submodule update --init --recursive
# docker exec -w /root/pymavlink -e MDEF=/root/mavlink/message_definitions $CONTAINER_ID python setup.py build install --user

# docker exec -w /root/MAVProxy $CONTAINER_ID git checkout 1.8.50
# docker exec -w /root/MAVProxy $CONTAINER_ID git submodule sync
# docker exec -w /root/MAVProxy $CONTAINER_ID git submodule update --init --recursive
# docker exec -w /root/MAVProxy $CONTAINER_ID python setup.py build install --user

# docker stop $CONTAINER_ID
