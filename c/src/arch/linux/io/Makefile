CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/linux/io

BUILD_DIR:=${BLOCKS_HOME}/c/build/arch/linux/io

all:
	@mkdir -p ${BUILD_DIR}
	gcc -c ${CFLAGS} serial.c -o ${BUILD_DIR}/serial.o

debug: CFLAGS+= -g
debug: all

clean:
	rm -f ${BUILD_DIR}/*.o
