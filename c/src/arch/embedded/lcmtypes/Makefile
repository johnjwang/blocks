CPATH:=${CURDIR}
HPATH:=${BLOCKS_HOME}/c/include/arch/embedded/lcmtypes

all:lcm-gen
	@mkdir -p ${BLOCKS_HOME}/lib/arch/embedded/lcmtypes/
	gcc -shared -fPIC ${CURDIR}/*.c -o ${BLOCKS_HOME}/lib/arch/embedded/liblcmtypes.so

lcm-gen:
	lcm-gen -c --cinclude=${HPATH} --c-cpath=${CPATH} --c-hpath=${HPATH} ${BLOCKS_HOME}/lcmtypes/*

clean:
	rm -rf ${CPATH}/*.c
	rm -rf ${HPATH}/*.h
	rm -rf ${BLOCKS_HOME}/lib/arch/embedded/lcmtypes
