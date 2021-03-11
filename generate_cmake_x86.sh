#!/usr/bin/env bash

trap 'exit -1' err

if [ -d build ]; then rm -rf build; fi

BASE_PATH=$PWD

TOOLCHAIN="-DCMAKE_TOOLCHAIN_FILE=$BASE_PATH/Toolchain_Linux_x86.cmake"

mkdir -p build/release
pushd build/release
cmake -DCMAKE_BUILD_TYPE=Release $TOOLCHAIN $BASE_PATH
popd

mkdir -p build/debug
pushd build/debug
cmake -DCMAKE_BUILD_TYPE=Debug $TOOLCHAIN $BASE_PATH
popd
