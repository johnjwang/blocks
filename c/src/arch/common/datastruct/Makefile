CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common/

BUILD_DIR:=${BLOCKS_HOME}/c/build/arch/common/datastruct

all:
	@mkdir -p ${BUILD_DIR}
	gcc -c circular.c ${CFLAGS} -o ${BUILD_DIR}/circular.o

debug: CFLAGS+= -g
debug: all

clean:
	rm -f ${BUILD_DIR}/*.o
