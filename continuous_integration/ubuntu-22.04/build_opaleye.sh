#!/usr/bin/env bash
trap 'exit -1' err

# docker buildx build --cache-to=type=local,dest=../cache --cache-from=type=local,src=../cache --platform linux/amd64 --output=type=image,push=true -t suburbanmarine/opaleye-ci:22.04 .
docker build -t suburbanmarine/opaleye-ci:22.04 .

CONTAINER_ID=$(docker create --rm -it suburbanmarine/opaleye-ci:22.04 /bin/bash)

docker start $CONTAINER_ID

docker exec -w /root/zcm $CONTAINER_ID git checkout patch_installer
docker exec -w /root/zcm $CONTAINER_ID git pull
docker exec -w /root/zcm $CONTAINER_ID ./scripts/make_debian_package.sh
docker exec -w /root/zcm $CONTAINER_ID gdebi -n /root/zcm/build/zcm_1.1.5_amd64.deb

docker exec -w /root/opaleye               $CONTAINER_ID git checkout master
docker exec -w /root/opaleye               $CONTAINER_ID git pull
docker exec -w /root/opaleye               $CONTAINER_ID ./generate_cmake_x86.sh
docker exec -w /root/opaleye/build/release $CONTAINER_ID make -j`nproc`
docker exec -w /root/opaleye/build/release $CONTAINER_ID make package

docker cp $CONTAINER_ID:/root/zcm/build/zcm_1.1.5_amd64.deb .
docker cp $CONTAINER_ID:/root/opaleye/build/release/opaleye-0.2.0-Linux.deb .

docker stop $CONTAINER_ID
