CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common/

LDFLAGS:=${BLOCKS_HOME}/c/lib/arch/libcommon.a -lpthread

BIN_DIR:=${BLOCKS_HOME}/bin/arch/common/io/test
BIN:=${BIN_DIR}/comms_test_linux

all:
	@rm -f bin
	@ln -sf ${BIN_DIR} ./bin
	@mkdir -p ${BLOCKS_HOME}/bin/arch/common/io/test
	gcc main.c ${LDFLAGS} ${CFLAGS} -o ${BIN}

debug: CFLAGS+=-g
debug: all

clean:
	rm -f ${BIN}
