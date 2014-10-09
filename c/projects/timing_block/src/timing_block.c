/*
 * timing_block.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */


#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"

#include "io/comms.h"

#include "bootloader.h"
#include "timing_block_util.h"
#include "time_util.h"
#include "uart_comms.h"
#include "usb_comms.h"
#include "timer_capture_generate.h"

#include "lcmtypes/channels_t.h"
#include "lcmtypes/kill_t.h"


#ifdef DEBUG
	#include "timing_block/debug.h"
#endif

static uint8_t killed = 0;

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
    bootloader_check_upload();

    //****** All user code goes below here ******

    uart_comms_up_init();
    timer_default_init();

    IntMasterEnable();

    usb_comms_init();

//	uart_comms_up_demo();
//	usb_demo();

	#ifdef DEBUG
		debug_init();
	#endif

	//
    // Loop forever.
    //

    static const int num_blinks = 3, num_leds = 2;
    static int start_idx = 1;
    int i = start_idx, next_i = start_idx, j = 1;
    
    uint8_t buf[40];
    uint16_t buflen = 40;
    int16_t channel_val[PPM_NUM_CHANNELS];

    comms_t *comms_out = comms_create(0);
    comms_add_publisher(comms_out, uart_comms_up_write_byte);
    comms_add_publisher(comms_out, usb_comms_write_byte);

    uart_comms_up_subscribe(CHANNEL_KILL, main_kill_msg_handler);
    uart_comms_up_subscribe(CHANNEL_CHANNELS, main_channels_msg_handler);

    while(1)
    {
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
            channel.num_channels = PPM_NUM_CHANNELS;
            uint8_t chan_i;
            for (chan_i=0; chan_i<PPM_NUM_CHANNELS; ++chan_i) {
                channel_val[chan_i] = timer_tics_to_us(
                        timer_default_read_pulse(ppm_channel_map[chan_i]));
            }
            channel.channels = channel_val;

            uint16_t max_len = __channels_t_encoded_array_size(&channel, 1);
            if(max_len > buflen) while(1);
            uint16_t len = __channels_t_encode_array(buf, 0, buflen, &channel, 1);

            comms_publish_blocking(comms_out, CHANNEL_CHANNELS, buf, len);


//    		uint8_t j;
//    		for (j=0; j<=TIMER_INPUT_9; ++j) {
//                uint32_t strlen = snprintf((char*)buf, buflen, "%lu ",
//                                           timer_tics_to_us(timer_default_pulse(j, 0)));
//                usb_comms_write(buf, strlen);
//    		}
//    		usb_comms_write_byte('\r');
//    		usb_comms_write_byte('\n');
		}

		#ifdef DEBUG
			debug();
		#endif
    }
}

static void main_kill_msg_handler(uint8_t *msg, uint16_t msg_len)
{
    kill_t kill;
    memset(&kill, 0, sizeof(kill));
    if (__kill_t_decode_array(msg, 0, msg_len, &kill, 1) >= 0) {
        timer_default_disconnect_all();
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
        timer_default_disconnect_all();

        // XXX: right now assuming you get 8 channels which are the outputs in order
        if (killed == 0) {
            uint8_t i;
            for (i=0; i<channels.num_channels; ++i) {
                timer_default_pulse(i + TIMER_OUTPUT_1, timer_us_to_tics(channels.channels[i]));
            }
        }
    }
    __channels_t_decode_array_cleanup(&channels, 1);
}
