CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/common -I${BLOCKS_HOME}/c/include/arch/linux -I${CURDIR}/ `pkg-config --cflags lcm`
LDFLAGS:=`pkg-config --libs lcm`

all:
	@rm bin
	@ln -sf ${BLOCKS_HOME}/bin ./bin
	@mkdir -p ${BLOCKS_HOME}/bin/projects/comms_driver
	gcc ${BLOCKS_HOME}/c/src/arch/linux/io/serial.c main.c ${BLOCKS_HOME}/c/src/arch/linux/lcmtypes/* ${BLOCKS_HOME}/c/src/arch/common/io/comms.c ${CFLAGS} ${LDFLAGS} -o ${BLOCKS_HOME}/bin/projects/comms_driver/comms_driver

debug: CFLAGS+=-g
debug: all

clean:
	rm -rf ${BLOCKS_HOME}/bin/projects/comms_driver
