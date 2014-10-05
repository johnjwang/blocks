#!/bin/bash

if [ "$1" == "" ]; then
    echo "Usage: $0 <blocks_home>"
    exit 1
fi

rm -f $1/lcm/*_t.c $1/lcm/include/*_t.h
/usr/local/bin/lcm-gen -c --c-no-pubsub --c-cpath=$1/lcm --c-hpath=$1/lcm/include $1/lcmtypes/*
