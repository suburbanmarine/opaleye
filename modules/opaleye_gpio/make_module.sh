#!/bin/bash

trap 'exit -1' err

make -C /lib/modules/`uname -r`/build M=$PWD clean
make -C /lib/modules/`uname -r`/build M=$PWD modules

exit 0
