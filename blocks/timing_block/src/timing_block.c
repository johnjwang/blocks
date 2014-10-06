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
#include "ppm.h"
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
    bootloader_check_upload();

    //****** All user code goes below here ******

    uart_comms_up_init();
//    gpio_ctl_init();
//    ppm_init();
    timer_default_init();
//    timer_capture_generate_init();

    IntMasterEnable();

    usb_init();

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
    
//    uint8_t buf[30];
//    uint32_t buflen = 30;
//    uint32_t outstart = GPIO_INPUT_1;
//    uint32_t output = outstart;
//    uint8_t outval = 1;

//    ppm_start();
//    timer_capture_generate_start();

//    timer_generate_pulse(2000);

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

//    	uint64_t utime = timestamp_now();
//    	static uint64_t last_utime = 0;
//    	if(last_utime + 300000 < utime)
//    	{
//    		last_utime = utime;
//    		gpio_ctl_write(output++, outval);
//			if (output > GPIO_OUTPUT_9) {
//				output = outstart;
//				outval = 1 - outval;
//			}
//
//			uint32_t strlen =gpio_ctl_values_snprintf_no_preread(buf, buflen);
//			usb_write(buf, strlen);
//		}

		#ifdef DEBUG
			debug();
		#endif
    }
}
