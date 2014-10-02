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


#include "bootloader.h"
#include "timing_block_util.h"
#include "time_util.h"
#include "uart_comms.h"
#include "usb_comms.h"
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

    IntMasterEnable();

    bootloader_check_upload();

    usb_init();
    timer_capture_generate_init();


	#ifdef DEBUG
		debug_init();
	#endif

	uint32_t sysclk = SysCtlClockGet();

	while(1){
		timer_generate_pulse(1000);
	}


	//
    // Loop forever.
    //
    while(1)
    {

    	static const int num_blinks = 3, num_leds = 2;
    	static int i = 0, next_i = 0, j = 1;
    	if(!is_blinking(i+1))
    	{
    		i = next_i;
    		blink_led(i+1, j);
    		j++;
    		if(j > num_blinks){
    			next_i = (i + 1) % num_leds;
				j = 1;
			}
    	}

		#ifdef DEBUG
			debug();
		#endif
    }
}
