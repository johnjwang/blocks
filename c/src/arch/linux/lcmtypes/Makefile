CPATH:=${CURDIR}
HPATH:=${BLOCKS_HOME}/include/arch/linux/lcmtypes

all:lcm-gen
	gcc -shared ${CURDIR}/*.c -o liblcmtypes.so

lcm-gen:
	lcm-gen -c --c-cpath=${CPATH} --c-hpath=${HPATH} ${BLOCKS_HOME}/lcmtypes/*

clean:
	rm -rf ${CPATH}/*.c
	rm -rf ${HPATH}/*.h
