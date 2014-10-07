#!/bin/bash

if [ "$1" == "" ]; then
    echo "USAGE: $0 <libcommon,libembedded,liblinux,libtiva'>"
    exit 1
fi

lib=$1

echo "Pushing $lib"
scp -P 49552 $BLOCKS_HOME/c/lib/arch/$lib.a blocks@bendeshome.com:lib/$lib.a
if [ "$?" != "0" ]; then
    echo "Unable to push $lib"
fi
