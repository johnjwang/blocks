#!/bin/bash

echo "Pushing embedded lcmtypes"
scp -r -P 49552 $BLOCKS_HOME/c/src/arch/embedded/lcmtypes/*_t.c blocks@bendeshome.com:src/embedded_lcmtypes/
scp -r -P 49552 $BLOCKS_HOME/c/include/arch/embedded/lcmtypes/*_t.h blocks@bendeshome.com:include/embedded_lcmtypes/
if [ "$?" != "0" ]; then
    echo "Unable to push embedded lcmtypes"
fi

echo ""

echo "Pushing linux lcmtypes"
scp -r -P 49552 $BLOCKS_HOME/c/src/arch/linux/lcmtypes/*_t.c blocks@bendeshome.com:src/linux_lcmtypes/
scp -r -P 49552 $BLOCKS_HOME/c/include/arch/linux/lcmtypes/*_t.h blocks@bendeshome.com:include/linux_lcmtypes/
if [ "$?" != "0" ]; then
    echo "Unable to push linux lcmtypes"
fi
