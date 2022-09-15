#!/usr/bin/env bash
trap 'exit -1' err

pushd l4t-r32.5.0
./build_opaleye.sh
popd

pushd l4t-r32.6.1
./build_opaleye.sh
popd

pushd ubuntu-18.04
./build_opaleye.sh
popd

pushd ubuntu-20.04
./build_opaleye.sh
popd

pushd ubuntu-22.04
./build_opaleye.sh
popd
