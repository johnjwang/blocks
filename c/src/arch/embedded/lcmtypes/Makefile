CFLAGS:=-I${BLOCKS_HOME}/c/include/arch/embedded

CPATH:=${CURDIR}
HPATH:=${BLOCKS_HOME}/c/include/arch/embedded/lcmtypes


BUILD_DIR:=${BLOCKS_HOME}/c/build/arch/embedded/lcmtypes

all:lcm-gen
	@mkdir -p ${BUILD_DIR}
	gcc -c ${CFLAGS} channels_t.c -o  ${BUILD_DIR}/channels_t.o
	gcc -c ${CFLAGS} imu_data_t.c -o  ${BUILD_DIR}/imu_data_t.o
	gcc -c ${CFLAGS} kill_t.c -o      ${BUILD_DIR}/kill_t.o
	gcc -c ${CFLAGS} rpms_t.c -o      ${BUILD_DIR}/rpms_t.o
	gcc -c ${CFLAGS} telemetry_t.c -o ${BUILD_DIR}/telemetry_t.o
	gcc -c ${CFLAGS} cfg_usb_serial_num_t.c -o ${BUILD_DIR}/cfg_usb_serial_num_t.o
	gcc -c ${CFLAGS} cfg_data_frequency_t.c -o ${BUILD_DIR}/cfg_data_frequency_t.o

debug: CFLAGS+=-g
debug: all

lcm-gen:
	lcm-gen -c --c-no-pubsub --cinclude=lcmtypes --c-cpath=${CPATH} --c-hpath=${HPATH} ${BLOCKS_HOME}/lcmtypes/*

clean:
	rm -rf ${CPATH}/*.c
	rm -rf ${HPATH}/*.h
	rm -rf ${BUILD_DIR}/*.o
