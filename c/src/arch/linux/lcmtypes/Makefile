CPATH:=${CURDIR}
HPATH:=${BLOCKS_HOME}/c/include/arch/linux/lcmtypes

all:lcm-gen
	@mkdir -p ${BLOCKS_HOME}/lib/arch/linux/lcmtypes/
	gcc -shared -fPIC ${CURDIR}/*.c -o ${BLOCKS_HOME}/lib/arch/linux/liblcmtypes.so

lcm-gen:
	lcm-gen -c --cinclude=${HPATH} --c-cpath=${CPATH} --c-hpath=${HPATH} ${BLOCKS_HOME}/lcmtypes/*

clean:
	rm -rf ${CPATH}/*.c
	rm -rf ${HPATH}/*.h
	rm -rf ${BLOCKS_HOME}/lib/arch/linux/lcmtypes
