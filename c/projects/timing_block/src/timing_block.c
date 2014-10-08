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

	uint8_t k;
	for (k=TIMER_OUTPUT_1; k<=TIMER_OUTPUT_8; ++k) {
        timer_default_pulse_RC(k, ((uint32_t)(k - TIMER_OUTPUT_1) * (uint32_t)UINT16_MAX) / 10);
	}

	//
    // Loop forever.
    //

    static const int num_blinks = 3, num_leds = 2;
    static int start_idx = 1;
    int i = start_idx, next_i = start_idx, j = 1;
    
    uint8_t buf[40];
    uint16_t buflen = 40;
    int16_t channel_val[PPM_NUM_CHANNELS];

    comms_t *comms_out = comms_create(256);
    comms_add_publisher(comms_out, uart_comms_up_write_byte);
    comms_add_publisher(comms_out, usb_comms_write_byte);

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

    		channels_t channel;
            channel.utime = utime;
            channel.num_channels = PPM_NUM_CHANNELS;
            uint8_t chan_i;
            for (chan_i=0; chan_i<PPM_NUM_CHANNELS; ++chan_i) {
                channel_val[chan_i] = timer_tics_to_us(
                        timer_default_pulse(ppm_channel_map[chan_i], 0));
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
