/*
 * ppm.c
 *
 *  Created on: Oct 3, 2014
 *      Author: Jonathan
 */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

#include "usb_comms.h"

#include "ppm.h"


#define PPM_NUM_CHANNELS 8

static uint32_t sys_clk_cycles_per_second;
static uint32_t timer_cycles_per_overflow = UINT16_MAX;
static uint32_t ms_per_overflow = 60;
static uint32_t timer_cycles_per_second;
static uint32_t timer_prescaler;
static uint32_t CYCLES_300_US;
static uint32_t CYCLES_PPM_TOTAL_PERIOD;
static uint32_t ppm_total_period_time_left;

static uint32_t channels[PPM_NUM_CHANNELS];

static void ppm_interrupt_handler(void);

void ppm_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);  // Output 9 -- PPM output

	// Period and Prescalar Calculation
	sys_clk_cycles_per_second = SysCtlClockGet();
	timer_cycles_per_second = timer_cycles_per_overflow * (uint32_t)(1000.0 / ms_per_overflow + 0.5);
	timer_prescaler = sys_clk_cycles_per_second / timer_cycles_per_second;
	if (timer_prescaler > 255) timer_prescaler = 255;

	CYCLES_300_US = timer_cycles_per_second * 0.0003;
	CYCLES_PPM_TOTAL_PERIOD = timer_cycles_per_second * 0.0225;
	ppm_total_period_time_left = CYCLES_PPM_TOTAL_PERIOD;

	int8_t i;
	for(i = 0; i < PPM_NUM_CHANNELS; ++i)
	{
	    channels[i] = timer_cycles_per_second * 0.0015;
	}

	GPIOPinTypeGPIOOutput(GPIO_PORTG_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, GPIO_PIN_0);

	TimerConfigure(TIMER4_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT );
	TimerPrescaleSet(TIMER4_BASE, TIMER_B, timer_prescaler);
	TimerLoadSet(TIMER4_BASE, TIMER_B, 1);

	// Register Capture Interrupt
	TimerIntRegister(TIMER4_BASE, TIMER_B, &ppm_interrupt_handler);

	// Prioritize Capture Interrupt
	IntPrioritySet(INT_TIMER4B, 0x20);

	// Enable Capture Interrupt
	IntEnable(INT_TIMER4B);
	TimerIntEnable(TIMER4_BASE, TIMER_TIMB_TIMEOUT);
}

void ppm_start(void)
{
	// Enable Timer
	TimerEnable(TIMER4_BASE, TIMER_B);
}

static void ppm_interrupt_handler(void)
{
#define NUM_STATES

#define START_PULSE 0
#define END_PULSE 1

	TimerIntClear(TIMER4_BASE, TIMER_TIMB_TIMEOUT);

	static int8_t pulse_state = 0, channel_num = 0;
    static uint32_t ticks;
	switch(pulse_state)
	{
	    case START_PULSE:
	        GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, 0);
	        TimerLoadSet(TIMER4_BASE, TIMER_B, CYCLES_300_US);
	        ppm_total_period_time_left -= CYCLES_300_US;
	        pulse_state = END_PULSE;
	        break;

	    case END_PULSE:
	        if(channel_num < PPM_NUM_CHANNELS)
	        {
	            ticks = channels[channel_num];
	            ppm_total_period_time_left -= channels[channel_num];
	            ++channel_num;
	        }
	        else
	        {
	            ticks = ppm_total_period_time_left;
	            ppm_total_period_time_left = CYCLES_PPM_TOTAL_PERIOD;
	            channel_num = 0;
	        }
            GPIOPinWrite(GPIO_PORTG_BASE, GPIO_PIN_0, GPIO_PIN_0);
            TimerLoadSet(TIMER4_BASE, TIMER_B, ticks);
            pulse_state = START_PULSE;
	        break;

		default:
			break;
	}

	TimerEnable(TIMER4_BASE, TIMER_B);

#undef END_PULSE
#undef START_PULSE

#undef NUM_STATES
}
