#!/bin/bash


if [ -a $BLOCKS_HOME/java/commons-collections-3.2.1.jar ]; then
    exit
fi

wget http://apache.osuosl.org//commons/collections/binaries/commons-collections-3.2.1-bin.tar.gz /tmp/commons-collections-3.2.1-bin.tar.gz
cd temp
tar -xvaf /tmp/commons-collections-3.2.1-bin.tar.gz
mv commons-collections-3.2.1-bin.tar.gz/commons-collections-3.2.1.jar $BLOCKS_HOME/java

echo "Add the following to you bashrc"
echo "export CLASSPATH=$CLASSPATH:$BLOCKS_HOME/java/commons-collections-3.2.1.jar"
