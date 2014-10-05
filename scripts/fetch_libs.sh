#!/bin/bash

libs[0] = "lcm"
libs[1] = "driverlib"
libs[2] = "usblib"

for lib in "$libs[@]"; do
    rsync -avtP blocks@bendeshome.com:libs/$lib.lib $BLOCKS_HOME/$lib/
done
