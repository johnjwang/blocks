/*
 * timing_block_util.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#include <stdbool.h>
#include <stdint.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "timing_block_util.h"
#include "time_util.h"

#define NUM_LEDS 3

static void TimingBlockUtilIntHandler(void);

static void set_led(uint8_t led_num, uint8_t led_status);

struct led_status
{
	uint8_t invert;
	volatile uint8_t led_status;
	volatile uint8_t num_toggles_left;
};

static struct led_status statuses[NUM_LEDS] = {0};

static uint64_t timer_cycles_per_overflow;

void leds_init()
{
    //**Initialize LEDs**//
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	SysCtlDelay(1);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_1);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_2);
	GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3);


    statuses[1].invert = 1;
    statuses[2].invert = 1;

    uint8_t i;
    for(i = 0; i < NUM_LEDS; ++i)
    	set_led(i, statuses[i].led_status);


    //	frequency    			  = SysCtlClockGet();
	timer_cycles_per_overflow = SysCtlClockGet() / 4;

	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER4);
	TimerDisable(WTIMER4_BASE, TIMER_A);
	TimerConfigure(WTIMER4_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC | TIMER_CFG_B_PERIODIC);

	TimerControlStall(WTIMER4_BASE, TIMER_A, true);

	TimerLoadSet(WTIMER4_BASE, TIMER_A, timer_cycles_per_overflow);

	TimerIntRegister(WTIMER4_BASE, TIMER_A, &TimingBlockUtilIntHandler);

	IntPrioritySet(INT_WTIMER4A, 0x20);

	IntEnable(INT_WTIMER4A);
	TimerIntEnable(WTIMER4_BASE, TIMER_TIMA_TIMEOUT);

	TimerEnable(WTIMER4_BASE, TIMER_A);
}

bool is_blinking(uint8_t led_num)
{
	if(statuses[led_num].num_toggles_left != 0) return true;
	return false;
}

bool blink_led(uint8_t led_num, uint8_t num_blinks)
{
	if(led_num >= NUM_LEDS) return false;

	if(is_blinking(led_num)) return false;

	statuses[led_num].num_toggles_left = num_blinks * 2 + 2;
	return true;
}

bool turn_on_led(uint8_t led_num)
{
	if(led_num >= NUM_LEDS) return false;

	if(is_blinking(led_num)) return false;

	set_led(led_num, 1);
	return true;
}

bool turn_off_led(uint8_t led_num)
{
	if(led_num >= NUM_LEDS) return false;

	if(is_blinking(led_num)) return false;

	set_led(led_num, 0);
	return true;
}

bool toggle_led(uint8_t led_num)
{
	if(led_num >= NUM_LEDS) return false;

	if(is_blinking(led_num)) return false;

	statuses[led_num].led_status ^= ~0;

	set_led(led_num, statuses[led_num].led_status);
	return true;
}

static void set_led(uint8_t led_num, uint8_t led_status)
{
	uint8_t pin_status = statuses[led_num].invert ? ~led_status : led_status;

	switch(led_num)
	{
		case 0:
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, pin_status);
			break;

		case 1:
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, pin_status);
			break;

		case 2:
			GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3, pin_status);
			break;

		default:
			break;
	}

	if(led_num < NUM_LEDS)
		statuses[led_num].led_status = led_status;
}

//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
static void TimingBlockUtilIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    TimerIntClear(WTIMER4_BASE, TIMER_TIMA_TIMEOUT);

    uint8_t i;
    for(i = 0; i < NUM_LEDS; ++i)
    {
    	if(statuses[i].num_toggles_left > 0)
    	{
    		if(statuses[i].num_toggles_left > 2)
    		{
				set_led(i, ~statuses[i].led_status);
    		}
    		statuses[i].num_toggles_left--;
    	}
    }
}
