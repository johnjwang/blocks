/*
 * timer.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"

static void TimeUtilIntHandler(void);

volatile int32_t uptime_overflow = 0;

static uint64_t frequency;
static uint64_t timer_cycles_per_overflow;
static uint64_t timer_period_us;
static bool init = false;

void time_init()
{
	if(init) return;
	init = true;

	frequency    			  = SysCtlClockGet();
	timer_cycles_per_overflow = SysCtlClockGet();
	timer_period_us			  = timer_cycles_per_overflow * 1000000.0 / frequency;

    SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER4);
    TimerConfigure(WTIMER4_BASE, TIMER_CFG_B_PERIODIC);

    TimerLoadSet(WTIMER4_BASE, TIMER_B, timer_cycles_per_overflow);

    TimerIntRegister(WTIMER4_BASE, TIMER_B, &TimeUtilIntHandler);

    IntPrioritySet(INT_WTIMER4B, 0x00);

    IntEnable(INT_WTIMER4B);
    TimerIntEnable(WTIMER4_BASE, TIMER_TIMB_TIMEOUT);

	TimerEnable(WTIMER4_BASE, TIMER_B);
}

uint64_t timestamp_now()
{
	uint64_t us_sec, us_sec_confirm, us_us;

	us_sec = uptime_overflow * timer_period_us;

	uint64_t ticks = TimerValueGet(WTIMER4_BASE, TIMER_B);

	us_sec_confirm = uptime_overflow * timer_period_us;

	if(us_sec != us_sec_confirm)
	{
		ticks = TimerValueGet(WTIMER4_BASE, TIMER_B);
		us_sec = uptime_overflow * timer_period_us;
	}

	if(us_sec != us_sec_confirm) return 0;

	us_us = ((uint64_t)((timer_cycles_per_overflow - ticks) * 1000000) / frequency);

    uint64_t time = us_sec + us_us;

    return time;
}

//*****************************************************************************
//
// The interrupt handler for the first timer interrupt.
//
//*****************************************************************************
static void TimeUtilIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
	uint32_t mode = TimerIntStatus(WTIMER4_BASE, 1);
    TimerIntClear(WTIMER4_BASE, mode);

    uptime_overflow += 1;

}
