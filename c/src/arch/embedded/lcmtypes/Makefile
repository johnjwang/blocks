CPATH:=${CURDIR}
HPATH:=${BLOCKS_HOME}/c/include/arch/embedded/lcmtypes

LIBDIR:=${BLOCKS_HOME}/c/lib/arch/embedded
LIB:=${LIBDIR}/liblcmtypes.a

all:lcm-gen
	@mkdir -p ${LIBDIR}
	gcc -shared -fPIC ${CFLAGS} ${CURDIR}/*.c -o ${LIB}

debug: CFLAGS+=-g
debug: all

lcm-gen:
	lcm-gen -c --cinclude=${HPATH} --c-cpath=${CPATH} --c-hpath=${HPATH} ${BLOCKS_HOME}/lcmtypes/*

clean:
	rm -rf ${CPATH}/*.c
	rm -rf ${HPATH}/*.h
	rm -rf ${LIB}
