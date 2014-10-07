#!/bin/bash

if [ "$1" == "" ]; then
    echo "USAGE: $0 <libcommon,libembedded,liblinux,libtiva'>"
    exit 1
fi

lib=$1

echo "Fetching $lib"
scp -P 49552 blocks@bendeshome.com:lib/$lib.a $BLOCKS_HOME/c/lib/arch/
if [ "$?" != "0" ]; then
    echo "Unable to fetch $lib"
fi
