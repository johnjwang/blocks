#!/bin/bash

DIR=$BLOCKS_HOME/java/skyspecs

mkdir -p $DIR/src/skyspecs/physics

mkdir -p $DIR/src/skyspecs/physics
ln -sf $SKYSPECS_HOME/java/src/skyspecs/physics/Quaternion.java $DIR/src/skyspecs/physics/

mkdir -p $DIR/src/skyspecs/util
ln -sf $SKYSPECS_HOME/java/src/skyspecs/util/ArrayUtil.java $DIR/src/skyspecs/util/

mkdir -p $DIR/src/skyspecs/vis
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/QuadModel.java $DIR/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/ArmModel.java $DIR/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/CoreModel.java $DIR/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/VzRing.java $DIR/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/VzVector.java $DIR/src/skyspecs/vis/

mkdir -p $DIR/src/skyspecs/math
ln -sf $SKYSPECS_HOME/java/src/skyspecs/math/MathUtil.java $DIR/src/skyspecs/math/


cd $BLOCKS_HOME/java/skyspecs
ant clean && ant
