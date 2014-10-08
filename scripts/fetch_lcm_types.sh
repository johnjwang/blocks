#!/bin/bash
echo "Fetching embedded lcmtypes"
mkdir -p $BLOCKS_HOME/c/include/arch/embedded/lcmtypes/
scp -r -P 49552 blocks@bendeshome.com:src/embedded_lcmtypes/* $BLOCKS_HOME/c/src/arch/embedded/lcmtypes/
scp -r -P 49552 blocks@bendeshome.com:include/embedded_lcmtypes/* $BLOCKS_HOME/c/include/arch/embedded/lcmtypes/
if [ "$?" != "0" ]; then
    echo "Unable to fetch embedded lcmtypes"
fi

echo ""

echo "Fetching linux lcmtypes"
mkdir -p $BLOCKS_HOME/c/include/arch/linux/lcmtypes/
scp -r -P 49552 blocks@bendeshome.com:src/linux_lcmtypes/* $BLOCKS_HOME/c/src/arch/linux/lcmtypes/
scp -r -P 49552 blocks@bendeshome.com:include/linux_lcmtypes/* $BLOCKS_HOME/c/include/arch/linux/lcmtypes/
if [ "$?" != "0" ]; then
    echo "Unable to fetch linux lcmtypes"
fi
