#!/bin/bash

libs=("lcm" "driverlib" "usblib")

for lib in "${libs[@]}"; do
    echo "Fetching Debug/$lib"
    scp -P 49552 blocks@bendeshome.com:lib/Debug/$lib.lib $BLOCKS_HOME/lib/$lib/Debug/$lib.lib
    if [ "$?" != "0" ]; then
        echo "Unable to push Debug/$lib"
    fi
    echo "Fetching Release/$lib"
    scp -P 49552 blocks@bendeshome.com:lib/Release/$lib.lib $BLOCKS_HOME/lib/$lib/Release/$lib.lib
    if [ "$?" != "0" ]; then
        echo "Unable to push Release/$lib"
    fi
done
