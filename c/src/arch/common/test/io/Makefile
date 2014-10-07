CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common/io

BIN:=${BLOCKS_HOME}/bin/arch/common/io/test/comms_test_linux

all:
	@rm -f bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@mkdir -p ${BLOCKS_HOME}/bin/arch/common/io/test
	gcc main.c ${BLOCKS_HOME}/c/src/arch/common/io/comms.c ${CFLAGS} -o ${BIN}

debug: CFLAGS+=-g
debug: all

clean:
	rm -f ${BIN}
