/*
 * timer_capture_generate.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

void timer_capture_generate_init(void)
{
	// Capture setup
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);
	//XXX This one's wrong on the board. An output pin is mapped to the same timer.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5);

	TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);

	TimerLoadSet(TIMER0_BASE, TIMER_A, timer_cycles_per_overflow);

	TimerIntRegister(TIMER0_BASE, TIMER_A, &Timer0IntAHandler);

	IntPrioritySet(INT_TIMER0A, 0x00);

	IntEnable(INT_TIMER0A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

	TimerEnable(TIMER0_BASE, TIMER_A);

	// Generate setup
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER1);

}
