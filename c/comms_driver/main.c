#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lcm/lcm.h"

#include "comms.h"
#include "serial.h"

#include "lcmtypes/telemetry_t.h"
#include "lcmtypes/imu_data_t.h"
#include "lcmtypes/rpms_t.h"

static uint64_t timestamp_now(void)
{
    struct timeval tv;
    gettimeofday (&tv, NULL);
    //XXX Not sure if this will work across hour boundaries
    return (uint64_t) tv.tv_sec * 1000000 + tv.tv_usec;
}
static bool publish_serial(uint8_t byte);
static void handler_kill(uint8_t *msg, uint16_t len);

comms_t *serial_comms;
serial_t *serial;

lcm_t *lcm;

int main()
{
    lcm = lcm_create(NULL);
    while(lcm == NULL)
    {
        fprintf(stderr, "LCM failed to initialize. Reattempting....\n");
        usleep(1000000);
        lcm = lcm_create(NULL);
    }

    char *dev_name = "/dev/stack";
    serial = serial_create(dev_name, B9600);
    if(serial == NULL)
    {
        fprintf(stderr, "Serial device does not exist at %s", dev_name);
        exit(1);
    }

    serial_comms = comms_create(publish_serial, 1000);

    comms_subscribe(serial_comms, CHANNEL_KILL, handler_kill);

    char data[1];
    while(true)
    {
        serial_read(serial, data, 1);
        comms_handle(serial_comms, data[0]);
    }

    comms_destroy(serial_comms);

    return 0;
}

static void handler_kill(uint8_t *msg, uint16_t len)
{
    printf("Received message on kill channel: ");
    uint16_t i;
    for(i = 0; i < len; ++i)
    {
        printf("%x ", msg[i]);
    }
    printf("\n");


    rpms_t rpms;
    rpms.utime = timestamp_now();
    rpms.num_rpms = len;
    rpms.rpms = (int16_t*) malloc(sizeof(int16_t) * len);
    for(i = 0; i < len; ++i)
    {
        rpms.rpms[i] = msg[i];
    }
    rpms_t_publish(lcm, "KILL", &rpms);
    free(rpms.rpms);
}

static bool publish_serial(uint8_t byte)
{
    static uint8_t buf[1];
    buf[0] = byte;
    return serial_write(serial, buf, 1);
}
