CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common/io

debug: CFLAGS+=-g
debug: all

all:
	@rm -f bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@mkdir -p ${BLOCKS_HOME}/bin/arch/common/io
	gcc main.c ${BLOCKS_HOME}/c/src/arch/common/io/comms.c ${CFLAGS} -o ${BLOCKS_HOME}/bin/arch/common/io/comms_test_linux

clean:
	rm -f ${BLOCKS_HOME}/c/bin/comms_test_linux
