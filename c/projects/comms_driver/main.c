#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>   // SIGINT and SIGTERM defines for signal interrupts
#include <errno.h>    // Error type definitions such as EINVAL or EBUSY
#include <pthread.h>

#include "lcm/lcm.h"

#include "io/comms.h"
#include "io/serial.h"

#include "lcmtypes/channels_t.h"
#include "lcmtypes/imu_data_t.h"
#include "lcmtypes/kill_t.h"
#include "lcmtypes/rpms_t.h"
#include "lcmtypes/telemetry_t.h"

static bool verbose = true;

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

static void* xbee_run(void*);
static void* usb_run(void*);
static bool publish_xbee(uint8_t byte);
static bool publish_usb(uint8_t byte);
static void handler_channels(uint8_t *msg, uint16_t len);
static void handler_kill(uint8_t *msg, uint16_t len);

pthread_cond_t exit_cond;
pthread_mutex_t exit_lock;

pthread_t xbee_thread;
comms_t *xbee_comms;
serial_t *xbee = NULL;

pthread_t usb_thread;
comms_t *usb_comms;
serial_t *usb = NULL;

lcm_t *lcm;

volatile bool done = false;
void interrupt(int sig)
{
    pthread_mutex_lock(&exit_lock);
        done = true;
        pthread_cond_broadcast(&exit_cond);
    pthread_mutex_unlock(&exit_lock);
}


int main(int argc, char *argv[])
{
    (void) signal(SIGINT, interrupt);
    (void) signal(SIGTERM, interrupt);

    pthread_cond_init(&exit_cond, NULL);
    pthread_mutex_init(&exit_lock, NULL);

    lcm = lcm_create(NULL);
    while(lcm == NULL)
    {
        fprintf(stderr, "LCM failed to initialize. Reattempting....\n");
        usleep(1000000);
        lcm = lcm_create(NULL);
    }


    // Create Serial Device for XBee
    char *xbee_dev_name = "/dev/XBee";
    if(argc > 1)
        xbee_dev_name = argv[1];
    xbee = serial_create(xbee_dev_name, B9600);
    if(xbee == NULL)
        fprintf(stderr, "XBee device does not exist at %s\n", xbee_dev_name);
    else
        fprintf(stdout, "XBee device successfully opened at 9600 baud on %s\n", xbee_dev_name);

    // Create Serial Device for USB
    char *usb_dev_name = "/dev/stack";
    if(argc > 2)
        usb_dev_name = argv[2];
    else
        fprintf(stdout, "Usb device successfully opened at %s\n", usb_dev_name);
    usb = serial_create(usb_dev_name, B115200);
    if(usb == NULL)
        fprintf(stderr, "Usb device does not exist at 115200 baud on %s\n", usb_dev_name);


    // If no open comm ports, quit.
    if(!xbee && !usb)
    {
        fprintf(stderr, "Failed to open any comms devices. Exiting...\n");
        exit(1);
    }


    // Create comms devices
    if(xbee)
    {
        xbee_comms = comms_create(1000);
        comms_add_publisher(xbee_comms, publish_xbee);
        comms_subscribe(xbee_comms, CHANNEL_KILL, handler_kill);
        comms_subscribe(xbee_comms, CHANNEL_CHANNELS, handler_channels);
        pthread_create(&xbee_thread, NULL, xbee_run, NULL);
    }

    // Create comms devices
    if(usb)
    {
        usb_comms = comms_create(1000);
        comms_add_publisher(usb_comms, publish_usb);
        comms_subscribe(usb_comms, CHANNEL_KILL, handler_kill);
        comms_subscribe(usb_comms, CHANNEL_CHANNELS, handler_channels);
        pthread_create(&usb_thread, NULL, usb_run, NULL);
    }

    pthread_mutex_lock(&exit_lock);
        while(!done) pthread_cond_wait(&exit_cond, &exit_lock);
    pthread_mutex_unlock(&exit_lock);

    if(xbee)
    {
        comms_destroy(xbee_comms);
        serial_destroy(xbee);
    }

    if(usb)
    {
        comms_destroy(usb_comms);
        serial_destroy(usb);
    }

    lcm_destroy(lcm);

    return 0;
}


static void* xbee_run(void* arg)
{
    fprintf(stdout, "Starting xbee receive thread\n");
    char data[1];
    while(!done)
    {
        serial_read(xbee, data, 1);
        comms_handle(xbee_comms, data[0]);
    }
    return NULL;
}

static bool publish_xbee(uint8_t byte)
{
    static uint8_t buf[1];
    buf[0] = byte;
    return serial_write(xbee, buf, 1);
}

static void* usb_run(void* arg)
{
    fprintf(stdout, "Starting usb receive thread\n");
    char data[1];
    while(!done)
    {
        serial_read(usb, data, 1);
        comms_handle(usb_comms, data[0]);
    }
    return NULL;
}

static bool publish_usb(uint8_t byte)
{
    static uint8_t buf[1];
    buf[0] = byte;
    return serial_write(usb, buf, 1);
}

static void handler_kill(uint8_t *msg, uint16_t len)
{
    verbose_printf("Received message on kill channel: ");
    uint16_t i;
    for(i = 0; i < len; ++i)
    {
        verbose_printf("%x ", msg[i]);
    }
    verbose_printf("\n");

    kill_t kill;
    memset(&kill, 0, sizeof(kill_t));
    __kill_t_decode_array(msg, 0, len, &kill, 1);
    kill_t_publish(lcm, "KILL_RX", &kill);
    kill_t_decode_cleanup(&kill);
}

static void handler_channels(uint8_t *msg, uint16_t len)
{
    verbose_printf("Received message on channels channel: ");
    uint16_t i;
    for(i = 0; i < len; ++i)
    {
        verbose_printf("%x ", msg[i]);
    }
    verbose_printf("\n");

    channels_t channels;
    memset(&channels, 0, sizeof(channels_t));
    __channels_t_decode_array(msg, 0, len, &channels, 1);
    channels_t_publish(lcm, "CHANNELS_RX", &channels);
    channels_t_decode_cleanup(&channels);
}
