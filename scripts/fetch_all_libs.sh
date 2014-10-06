#!/bin/bash

libs=("lcm" "driverlib" "usblib")

for lib in "${libs[@]}"; do
    $BLOCKS_HOME/scripts/fetch_lib.sh $lib
done
