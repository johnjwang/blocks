CFLAGS:=-I${BLOCKS_HOME}/c/common/include -I${BLOCKS_HOME}/c/comms_linux_test

debug: CFLAGS+=-g
debug: all

all:
	gcc main.c ${BLOCKS_HOME}/c/common/src/comms.c ${CFLAGS} -o comms_linux_test

clean:
	rm -f comms_linux_test
