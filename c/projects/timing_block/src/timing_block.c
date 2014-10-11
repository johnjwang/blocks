/*
 * timing_block.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"

#include "io/comms.h"

#include "bootloader.h"
#include "stack.h"
#include "time_util.h"
#include "timer_capture_generate.h"
#include "timing_block_util.h"
#include "uart_comms.h"
#include "usb_comms.h"
#include "watchdog.h"

#include "lcmtypes/channels_t.h"
#include "lcmtypes/kill_t.h"


#ifdef DEBUG
	#include "timing_block/debug.h"
#endif

static uint8_t killed = 0;

#define AUTONOMY_CMD_TIMEOUT_US 200000
static uint8_t autonomous_ready = 0;
static uint64_t last_autonomy_cmd_utime = 0;

static void main_manual_auto_switch_handler(uint32_t pulse_us);
static void main_kill_msg_handler(uint8_t *msg, uint16_t msg_len);
static void main_channels_msg_handler(uint8_t *msg, uint16_t msg_len);

int main(void)
{
    //
    // Set the clocking to run from the PLL at 80 MHz.
    //
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    IntMasterDisable();
    leds_init();
    time_init();
    stack_enumerate(A, 2, B, 3);
    bootloader_check_upload();
    IntMasterEnable();

    //****** All user code dependent on interrupts gets initialzed here *******
    usb_comms_init();
    uart_comms_up_init();
//    timer_default_init();
    watchdog_init(SysCtlClockGet() / 10);

//	uart_comms_up_demo();
//	usb_demo();

	#ifdef DEBUG
		debug_init();
	#endif

	//
    // Loop forever.
    //

    comms_t *comms_out = comms_create(0);
    comms_add_publisher(comms_out, uart_comms_up_write_byte);
    comms_add_publisher(comms_out, usb_comms_write_byte);

    uart_comms_up_subscribe(CHANNEL_KILL, main_kill_msg_handler);
    uart_comms_up_subscribe(CHANNEL_CHANNELS, main_channels_msg_handler);
    usb_comms_subscribe(CHANNEL_KILL, main_kill_msg_handler);
    usb_comms_subscribe(CHANNEL_CHANNELS, main_channels_msg_handler);

//    timer_register_switch_monitor(main_manual_auto_switch_handler);

    static const int num_blinks = 3, num_leds = 2;
    static int start_idx = 1;
    int i = start_idx, next_i = start_idx, j = 1;

    uint8_t buf[50];
    uint16_t buflen = 50;
    int16_t channel_val[NUM_TIMERS - 2];

    while(1)
    {
        watchdog_feed();
    	if(!is_blinking(i))
    	{
    		i = next_i;
    		blink_led(i, j);
    		j++;
    		if(j > num_blinks){
    			next_i = (i + 1 - start_idx) % num_leds + start_idx;
				j = 1;
			}
    	}

    	uint64_t utime = timestamp_now();
    	static uint64_t last_utime = 0;
    	if(last_utime + 100000 < utime)
    	{
    		last_utime = utime;

    		// XXX: Need to send ALL channels (inputs and outputs)
    		channels_t channel;
            channel.utime = utime;
            channel.num_channels = NUM_TIMERS - 2;
            uint8_t chan_i, timer_ind;
            for (chan_i=0; chan_i<channel.num_channels; ++chan_i) {
                if (chan_i <= TIMER_INPUT_8) timer_ind = chan_i;
                else                         timer_ind = chan_i - (TIMER_INPUT_8 + 1) + TIMER_OUTPUT_1;

//                channel_val[chan_i] = timer_tics_to_us(
//                        timer_default_read_pulse(timer_ind));
            }
            channel.channels = channel_val;
            // XXX: had a very weird but where receiving too much or something basically
            //      broke all of the channels, I think it was because we were receiving
            //      on both the Xbee and the USB on the same time
            // XXX: Every now and then sometimes get a randome signal value on an output that
            //      is not receiving anything from the autonomy. I think it might be a leak
            //      through of the values from the input pins

            uint16_t max_len = __channels_t_encoded_array_size(&channel, 1);
            if(max_len > buflen) while(1);
            uint16_t len = __channels_t_encode_array(buf, 0, buflen, &channel, 1);

            comms_publish_blocking(comms_out, CHANNEL_CHANNELS, buf, len);
		}

		#ifdef DEBUG
			debug();
		#endif
    }
}

static void main_manual_auto_switch_handler(uint32_t pulse_us)
{
    uint64_t utime = timestamp_now();

    // Autonomous cmds have timed out
    if (utime - last_autonomy_cmd_utime >= AUTONOMY_CMD_TIMEOUT_US) {
        if (autonomous_ready == 1 && killed != 1) timer_default_reconnect_all();
        autonomous_ready = 0;
        return;
    }

    if (pulse_us <= 1500) {
        if (autonomous_ready == 1 && killed != 1) timer_default_reconnect_all();
        autonomous_ready = 0;
    } else {
        autonomous_ready = 1;
    }
}

static void main_kill_msg_handler(uint8_t *msg, uint16_t msg_len)
{
    kill_t kill;
    memset(&kill, 0, sizeof(kill));
    if (__kill_t_decode_array(msg, 0, msg_len, &kill, 1) >= 0) {
        timer_default_disconnect_all();
        // XXX: this will actually flatline after 200 ms due to sigtimeout, may need to check
        //      if that is safe
        timer_default_pulse_allpwm(timer_us_to_tics(1000));
        killed = 1;
    }
    __kill_t_decode_array_cleanup(&kill, 1);
}

static void main_channels_msg_handler(uint8_t *msg, uint16_t msg_len)
{
    channels_t channels;
    memset(&channels, 0, sizeof(channels));
    if (__channels_t_decode_array(msg, 0, msg_len, &channels, 1) >= 0) {

        // Ensure we are autonomous enabled
        last_autonomy_cmd_utime = timestamp_now();
        if (autonomous_ready == 1) {
            timer_default_disconnect_all();

            // XXX: right now assuming you get 8 channels which are the outputs in order
            if (killed == 0) {
                uint8_t i;
                for (i=0; i<channels.num_channels; ++i) {
                    timer_default_pulse(i + TIMER_OUTPUT_1, timer_us_to_tics(channels.channels[i]));
                }
            }
        }
    }
    __channels_t_decode_array_cleanup(&channels, 1);
}
