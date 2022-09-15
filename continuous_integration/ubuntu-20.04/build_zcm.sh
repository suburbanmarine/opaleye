docker build -t opaleye-ubuntu-20.04 .

CONTAINER_ID=$(docker create -it opaleye-ubuntu-20.04:latest /bin/bash)
docker start $CONTAINER_ID
docker cp $CONTAINER_ID:/root/zcm/build/zcm_1.1.5_amd64.deb .
docker stop $CONTAINER_ID