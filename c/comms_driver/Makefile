CFLAGS:=-I${BLOCKS_HOME}/c/common/include -I${BLOCKS_HOME}/c/comms_driver `pkg-config --cflags lcm`
LDFLAGS:=`pkg-config --libs lcm`

all:lcm-gen
	@ln -sf ${BLOCKS_HOME}/c/bin ${BLOCKS_HOME}/c/comms_driver/
	gcc serial.c main.c lcmtypes/* ${BLOCKS_HOME}/c/common/src/comms.c ${CFLAGS} ${LDFLAGS} -o ${BLOCKS_HOME}/c/bin/comms_driver

debug: CFLAGS+=-g
debug: all

lcm-gen:
	mkdir -p lcmtypes
	lcm-gen -c --c-cpath=./lcmtypes --c-hpath=./lcmtypes ${BLOCKS_HOME}/lcmtypes/*


clean:
	rm -f ${BLOCKS_HOME}/c/bin/comms_driver
	rm -rf lcmtypes
