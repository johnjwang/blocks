/*
 * timing_block.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */


#include <stdbool.h>
#include <stdint.h>

#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"


#include "bootloader.h"
#include "timing_block_util.h"
#include "time_util.h"
#include "uart_comms.h"
#include "usb_comms.h"
#include "gpio_ctl.h"
#include "timer_capture_generate.h"


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
    uart_comms_up_init();
    gpio_ctl_init();

    IntMasterEnable();

    bootloader_check_upload();

    usb_init();
//    timer_capture_generate_init();

//	uart_comms_up_demo();
//	usb_demo();

	#ifdef DEBUG
		debug_init();
	#endif

//	uint32_t sysclk = SysCtlClockGet();

//	uint32_t pulse_width = 50000;
//	while(1){
//		timer_generate_pulse(pulse_width);
//		SysCtlDelay(SysCtlClockGet() / 3);
//	}


	//
    // Loop forever.
    //

    static const int num_blinks = 3, num_leds = 2;
    static int start_idx = 1;
    int i = start_idx, next_i = start_idx, j = 1;
    
    uint8_t buf[30];
    uint32_t buflen = 30;
    uint32_t output = GPIO_OUTPUT_1;
    uint8_t outval = 1;

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
    	static last_utime = 0;
    	if(last_utime + 300000 > utime)
    	{
    		last_utime = utime;
    		gpio_ctl_write(output++, outval);
			if (output > GPIO_OUTPUT_9) {
				output = GPIO_OUTPUT_1;
				outval = 1 - outval;
			}

			gpio_ctl_values_snprintf(buf, buflen);
			usb_write(buf, buflen);
		}

		#ifdef DEBUG
			debug();
		#endif
    }
}
