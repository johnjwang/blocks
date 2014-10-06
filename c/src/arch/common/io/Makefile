CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common/io

LIBDIR:=${BLOCKS_HOME}/c/lib/arch/common
LIB:=${LIBDIR}/io.lib

all:
	@mkdir -p ${LIBDIR}
	gcc ./*.c ${CFLAGS} -shared -fPIC -o ${LIB}

debug: CFLAGS+= -g

clean:
	rm -f ${LIB}

