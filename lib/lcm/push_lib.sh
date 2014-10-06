#!/bin/bash

lib=`echo $(pwd)/$0 | sed "s/.*\/lib\/\([a-z,0-9]*\)\/.*/\1/"`

$BLOCKS_HOME/scripts/push_lib.sh $lib
