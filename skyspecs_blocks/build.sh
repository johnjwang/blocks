#!/bin/bash
mkdir -p $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/physics

mkdir -p $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/physics
ln -sf $SKYSPECS_HOME/java/src/skyspecs/physics/Quaternion.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/physics/

mkdir -p $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/util
ln -sf $SKYSPECS_HOME/java/src/skyspecs/util/ArrayUtil.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/util/

mkdir -p $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/vis
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/QuadModel.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/ArmModel.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/CoreModel.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/VzRing.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/vis/
ln -sf $SKYSPECS_HOME/java/src/skyspecs/vis/VzVector.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/vis/

mkdir -p $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/math
ln -sf $SKYSPECS_HOME/java/src/skyspecs/math/MathUtil.java $BLOCKS_HOME/skyspecs_blocks/src/skyspecs/math/


cd $BLOCKS_HOME/skyspecs_blocks
ant clean && ant
