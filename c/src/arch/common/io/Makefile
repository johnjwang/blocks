CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common/

BUILD_DIR:=${BLOCKS_HOME}/c/build/arch/common/io

all:
	@mkdir -p ${BUILD_DIR}
	gcc -c comms.c ${CFLAGS} -o ${BUILD_DIR}/comms.o

debug: CFLAGS+= -g
debug: all

clean:
	rm -f ${BUILD_DIR}/*.o
