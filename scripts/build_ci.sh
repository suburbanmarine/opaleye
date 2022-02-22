#!/usr/bin/env bash

trap 'exit -1' err

set -v

CONTAINER_ID=$(docker create -v $GITHUB_WORKSPACE:/tmp/workspace -it docker.pkg.github.com/suburbanmarine/opaleye/opaleye-ci:${GITHUB_REF##*/}  /bin/bash)
docker start $CONTAINER_ID
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID ./generate_cmake_x86.sh
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` package -C build/ram/debug
docker exec -u $(id -u):$(id -g) -w /tmp/workspace/ $CONTAINER_ID make -j`nproc` package -C build/ram/release
docker stop $CONTAINER_ID

pushd $GITHUB_WORKSPACE/build/debug
sha256sum -b *.deb *.tar.gz | tee sha256.txt
popd

pushd $GITHUB_WORKSPACE/build/release
sha256sum -b *.deb *.tar.gz | tee sha256.txt
popd