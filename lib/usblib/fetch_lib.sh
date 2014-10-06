#!/bin/bash

lib=`echo $(pwd)/$0 | sed "s/.*\/lib\/\([a-z,0-9]*\)\/.*/\1/"`
echo "$lib"

$BLOCKS_HOME/scripts/fetch_lib.sh $lib
