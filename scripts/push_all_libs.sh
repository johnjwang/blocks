#!/bin/bash

libs=("lcm" "driverlib" "usblib")

for lib in "${libs[@]}"; do
    $BLOCKS_HOME/scripts/push_lib.sh $lib
done
