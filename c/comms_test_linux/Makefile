CFLAGS:=-I${BLOCKS_HOME}/c/common/include -I${BLOCKS_HOME}/c/comms_linux_test

debug: CFLAGS+=-g
debug: all

all:
	@ln -sf ${BLOCKS_HOME}/c/bin ${BLOCKS_HOME}/c/comms_test_linux/
	gcc main.c ${BLOCKS_HOME}/c/common/src/comms.c ${CFLAGS} -o ${BLOCKS_HOME}/c/bin/comms_test_linux

clean:
	rm -f ${BLOCKS_HOME}/c/bin/comms_test_linux
