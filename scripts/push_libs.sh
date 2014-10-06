#!/bin/bash

libs=("lcm" "driverlib" "usblib")

for lib in "${libs[@]}"; do
    echo "Pushing Debug/$lib"
    scp -P 49552 $BLOCKS_HOME/lib/$lib/Debug/$lib.lib blocks@bendeshome.com:lib/Debug/$lib.lib
    if [ "$?" != "0" ]; then
        echo "Unable to push Debug/$lib"
    fi
    echo "Pushing Release/$lib"
    scp -P 49552 $BLOCKS_HOME/lib/$lib/Release/$lib.lib blocks@bendeshome.com:lib/Release/$lib.lib
    if [ "$?" != "0" ]; then
        echo "Unable to push Release/$lib"
    fi
done
