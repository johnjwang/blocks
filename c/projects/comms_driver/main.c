#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>   // SIGINT and SIGTERM defines for signal interrupts
#include <errno.h>    // Error type definitions such as EINVAL or EBUSY

#include "lcm/lcm.h"

#include "io/comms.h"
#include "io/serial.h"

#include "lcmtypes/kill_t.h"
#include "lcmtypes/imu_data_t.h"
#include "lcmtypes/rpms_t.h"
#include "lcmtypes/telemetry_t.h"

static bool verbose = false;

#define verbose_printf(...) \
    do{\
        if(verbose) printf(__VA_ARGS__);\
    }while(0)\


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

volatile bool done = false;
void interrupt(int sig)
{
    done = true;
}

int main()
{
    (void) signal(SIGINT, interrupt);
    (void) signal(SIGTERM, interrupt);

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
        fprintf(stderr, "Serial device does not exist at %s\n", dev_name);
        exit(1);
    }

    serial_comms = comms_create(1000);
    comms_add_publisher(serial_comms, publish_serial);

    comms_subscribe(serial_comms, CHANNEL_KILL, handler_kill);

    char data[1];
    while(!done)
    {
        serial_read(serial, data, 1);
        comms_handle(serial_comms, data[0]);
    }

    comms_destroy(serial_comms);
    lcm_destroy(lcm);

    return 0;
}

static void handler_kill(uint8_t *msg, uint16_t len)
{
    verbose_printf("Received message on kill channel: ");
    uint16_t i;
    for(i = 0; i < len; ++i)
    {
        verbose_printf("%x", msg[i]);
    }
    verbose_printf("\n");

    kill_t kill;
    memset(&kill, 0, sizeof(kill_t));
    __kill_t_decode_array(msg, 0, len, &kill, 1);
    kill_t_publish(lcm, "KILL_RX", &kill);
    kill_t_decode_cleanup(&kill);
}

static bool publish_serial(uint8_t byte)
{
    static uint8_t buf[1];
    buf[0] = byte;
    return serial_write(serial, buf, 1);
}
