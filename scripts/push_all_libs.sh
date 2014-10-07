#!/bin/bash

libs=("libcommon" "libembedded" "liblinux" "libtiva")

for lib in "${libs[@]}"; do
    $BLOCKS_HOME/scripts/push_lib.sh $lib
done
