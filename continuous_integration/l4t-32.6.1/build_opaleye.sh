#!/usr/bin/env bash
trap 'exit -1' err

docker buildx build --cache-to=registry --cache-from=registry --platform linux/arm64 --output=type=registry -t suburbanmarine/opaleye-ci:l4t-32.6.1 .

CONTAINER_ID=$(docker create -it suburbanmarine/opaleye-ci:l4t-32.5.0 /bin/bash)

docker start $CONTAINER_ID

docker exec -w /root/zcm $CONTAINER_ID git checkout patch_installer
docker exec -w /root/zcm $CONTAINER_ID ./scripts/make_debian_package.sh
docker exec -w /root/zcm $CONTAINER_ID gdebi -n /root/zcm/build/zcm_1.1.5_arm64.deb

docker exec -w /root/opaleye               $CONTAINER_ID git checkout master
docker exec -w /root/opaleye               $CONTAINER_ID ./generate_cmake_xaviernx.sh
docker exec -w /root/opaleye/build/release $CONTAINER_ID make -j`nproc`
docker exec -w /root/opaleye/build/release $CONTAINER_ID make package

docker cp $CONTAINER_ID:/root/zcm/build/zcm_1.1.5_arm64.deb .
docker cp $CONTAINER_ID:/root/opaleye/build/release/opaleye-0.2.0-Linux.deb .

docker stop $CONTAINER_ID