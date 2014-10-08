CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common -I${BLOCKS_HOME}/c/include/arch/linux `pkg-config --cflags lcm`
LDFLAGS:=${BLOCKS_HOME}/c/lib/arch/libcommon.a ${BLOCKS_HOME}/c/lib/arch/liblinux.a `pkg-config --libs lcm`

BINDIR:=${BLOCKS_HOME}/bin/projects
BIN:=${BINDIR}/comms_driver

all:
	@rm bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@mkdir -p ${BINDIR}
	gcc main.c ${CFLAGS} ${LDFLAGS} -o ${BIN}

debug: CFLAGS+=-g
debug: all

clean:
	rm -rf ${BIN}
