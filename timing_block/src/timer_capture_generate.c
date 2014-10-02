/*
 * timer_capture_generate.c
 *
 *  Created on: Oct 2, 2014
 *      Author: Isaac
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

#include "usb_comms.h"
#include "timer_capture_generate.h"

uint32_t sys_clk_cycles_per_second;
uint32_t timer_cycles_per_overflow = UINT16_MAX;
uint32_t ms_per_overflow = 60;
uint32_t ms_per_pwm_period = 20;
uint32_t timer_cycles_per_second;
uint32_t timer_prescaler;
uint32_t timer_generate_load;

#define TIMEOUT_MSG "cap timed out\r\n"
#define TIMEOUT_MSG_LEN sizeof(TIMEOUT_MSG)

#define CAPTURE_LOW_MSG "captured edge low\r\n"
#define CAPTURE_LOW_MSG_LEN sizeof(CAPTURE_LOW_MSG)

#define CAPTURE_HIGH_MSG "captured edge high\r\n"
#define CAPTURE_HIGH_MSG_LEN sizeof(CAPTURE_HIGH_MSG)

// XXX: put all the pin mux and timer maping into these and group by input / output #
//		this will let us do for loops for most of the tedious stuff
//struct timer_capture_t
//{
//
//};
//
//struct timer_generate_t
//{
//
//};

static void timer_capture_int_handler(void);

void timer_capture_generate_init(void)
{
	// GPIO port enable
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

	// Capture setup							   // CCP0      |  CCP1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // Input  4  |  Output 8
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  // Input  5  |  Input  6
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);  // Input  9  |  Input  8
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);  // Output 9  |  Output 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);  // Output 2  |  Output 3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0); // Output 4  |  Output 5
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER1); // Output 6  |  Output 7
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2); //           |  Input  3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3); // Input  1  |  Input  2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5); // Input  7  |

	// GPIO pin enable
	GPIOPinConfigure(GPIO_PD3_WT3CCP1); // Input 1
	GPIOPinConfigure(GPIO_PD2_WT3CCP0); // Input 2
	GPIOPinConfigure(GPIO_PD1_WT2CCP1); // Input 3
	GPIOPinConfigure(GPIO_PB6_T0CCP0);  // Input 4
	GPIOPinConfigure(GPIO_PB4_T1CCP0);  // Input 5
	GPIOPinConfigure(GPIO_PB5_T1CCP1);  // Input 6
	GPIOPinConfigure(GPIO_PD6_WT5CCP0); // Input 7
	GPIOPinConfigure(GPIO_PB3_T3CCP1);  // Input 8
	GPIOPinConfigure(GPIO_PB2_T3CCP0);  // Input 9
	GPIOPinConfigure(GPIO_PG1_T4CCP1);  // Output 1
	GPIOPinConfigure(GPIO_PG2_T5CCP0);  // Output 2
	GPIOPinConfigure(GPIO_PG3_T5CCP1);  // Output 3
	GPIOPinConfigure(GPIO_PC4_WT0CCP0); // Output 4
	GPIOPinConfigure(GPIO_PC5_WT0CCP1); // Output 5
	GPIOPinConfigure(GPIO_PC6_WT1CCP0); // Output 6
	GPIOPinConfigure(GPIO_PC7_WT1CCP1); // Output 7
	GPIOPinConfigure(GPIO_PB7_T0CCP1);  // Output 8
	GPIOPinConfigure(GPIO_PG0_T4CCP0);  // Output 9

	// GPIO pin type
	GPIOPinTypeTimer(GPIO_PORTB_BASE,                           GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	GPIOPinTypeTimer(GPIO_PORTC_BASE,                                                     GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	GPIOPinTypeTimer(GPIO_PORTD_BASE,              GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |                           GPIO_PIN_6             );
	GPIOPinTypeTimer(GPIO_PORTC_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3                                                    );


	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

	// Timer Configuration
	TimerConfigure(TIMER0_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_PWM     );
	TimerConfigure(TIMER1_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_CAP_TIME);
	TimerConfigure(TIMER3_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_CAP_TIME);
	TimerConfigure(TIMER4_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(TIMER5_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(WTIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR |                        TIMER_CFG_B_CAP_TIME);
	TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_CAP_TIME);
	TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME                       );

	// Period and Prescalar Calculation
	sys_clk_cycles_per_second = SysCtlClockGet();
	timer_cycles_per_second = timer_cycles_per_overflow * (uint32_t)(1000.0 / ms_per_overflow + 0.5);
	timer_prescaler = sys_clk_cycles_per_second / timer_cycles_per_second;
	timer_generate_load = (timer_cycles_per_overflow * ms_per_pwm_period) / ms_per_overflow;
	if (timer_prescaler > 255) timer_prescaler = 255;

	// Set Prescale Value
	TimerPrescaleSet(TIMER0_BASE,  TIMER_A, timer_prescaler);
	TimerPrescaleSet(TIMER0_BASE,  TIMER_B, timer_prescaler);
	TimerPrescaleSet(TIMER1_BASE,  TIMER_A, timer_prescaler);
	TimerPrescaleSet(TIMER1_BASE,  TIMER_B, timer_prescaler);
	TimerPrescaleSet(TIMER3_BASE,  TIMER_A, timer_prescaler);
	TimerPrescaleSet(TIMER3_BASE,  TIMER_B, timer_prescaler);
	TimerPrescaleSet(TIMER4_BASE,  TIMER_A, timer_prescaler);
	TimerPrescaleSet(TIMER4_BASE,  TIMER_B, timer_prescaler);
	TimerPrescaleSet(TIMER5_BASE,  TIMER_A, timer_prescaler);
	TimerPrescaleSet(TIMER5_BASE,  TIMER_B, timer_prescaler);
	TimerPrescaleSet(WTIMER0_BASE, TIMER_A, timer_prescaler);
	TimerPrescaleSet(WTIMER0_BASE, TIMER_B, timer_prescaler);
	TimerPrescaleSet(WTIMER1_BASE, TIMER_A, timer_prescaler);
	TimerPrescaleSet(WTIMER1_BASE, TIMER_B, timer_prescaler);
	TimerPrescaleSet(WTIMER2_BASE, TIMER_B, timer_prescaler);
	TimerPrescaleSet(WTIMER3_BASE, TIMER_A, timer_prescaler);
	TimerPrescaleSet(WTIMER3_BASE, TIMER_B, timer_prescaler);
	TimerPrescaleSet(WTIMER5_BASE, TIMER_A, timer_prescaler);

	// Set Load Value
	TimerLoadSet(TIMER0_BASE,  TIMER_A, timer_cycles_per_overflow);
	TimerLoadSet(TIMER0_BASE,  TIMER_B, timer_generate_load     );
	TimerLoadSet(TIMER1_BASE,  TIMER_A, timer_cycles_per_overflow);
	TimerLoadSet(TIMER1_BASE,  TIMER_B, timer_cycles_per_overflow);
	TimerLoadSet(TIMER3_BASE,  TIMER_A, timer_cycles_per_overflow);
	TimerLoadSet(TIMER3_BASE,  TIMER_B, timer_generate_load      );
	TimerLoadSet(TIMER4_BASE,  TIMER_A, timer_generate_load      );
	TimerLoadSet(TIMER4_BASE,  TIMER_B, timer_generate_load      );
	TimerLoadSet(TIMER5_BASE,  TIMER_A, timer_generate_load      );
	TimerLoadSet(TIMER5_BASE,  TIMER_B, timer_generate_load      );
	TimerLoadSet(WTIMER0_BASE, TIMER_A, timer_generate_load      );
	TimerLoadSet(WTIMER0_BASE, TIMER_B, timer_generate_load      );
	TimerLoadSet(WTIMER1_BASE, TIMER_A, timer_generate_load      );
	TimerLoadSet(WTIMER1_BASE, TIMER_B, timer_cycles_per_overflow);
	TimerLoadSet(WTIMER2_BASE, TIMER_B, timer_cycles_per_overflow);
	TimerLoadSet(WTIMER3_BASE, TIMER_A, timer_cycles_per_overflow);
	TimerLoadSet(WTIMER3_BASE, TIMER_B, timer_cycles_per_overflow);
	TimerLoadSet(WTIMER5_BASE, TIMER_A, timer_cycles_per_overflow);

	// Set PWM Generators to Update Match Value After Overflow
	TimerUpdateMode(TIMER0_BASE, TIMER_B, TIMER_UP_MATCH_TIMEOUT);

	// Set PWM to 0 Duty Cycle
	TimerMatchSet(TIMER0_BASE, TIMER_B, timer_generate_load      );

	// Configure Capture Event
	TimerControlEvent(TIMER0_BASE,  TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER1_BASE,  TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER1_BASE,  TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER3_BASE,  TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER3_BASE,  TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER3_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER5_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);

	// XXX stopped caring about all of them at once

	// Register Capture Interrupt
	TimerIntRegister(TIMER1_BASE, TIMER_A, &timer_capture_int_handler);

	// Prioritize Capture Interrupt
	IntPrioritySet(INT_TIMER1A, 0x20);

	// Enable Capture Interrupt
	IntEnable(INT_TIMER1A);
	TimerIntEnable(TIMER1_BASE, TIMER_CAPA_EVENT);

	// Enable Timer
	TimerEnable(TIMER1_BASE, TIMER_A);
	TimerEnable(TIMER0_BASE, TIMER_B);
}

void timer_generate_pulse(uint32_t pulse_width)
{
	uint32_t timer_match;
	if (pulse_width > timer_generate_load) timer_match = 0;
	else timer_match = timer_generate_load - pulse_width;

	TimerMatchSet(TIMER0_BASE, TIMER_B, timer_match);
}

static void timer_capture_int_handler(void)
{
	TimerIntClear(TIMER1_BASE, TIMER_CAPA_EVENT);

	static uint8_t msg[30];

	if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_4) == 0) {
		usb_write(CAPTURE_LOW_MSG, CAPTURE_LOW_MSG_LEN);
	} else {
//		usb_write(CAPTURE_HIGH_MSG, CAPTURE_HIGH_MSG_LEN);
	}
//	snprintf((char*)msg, 30, "%lu\r\n", TimerValueGet(TIMER1_BASE, TIMER_A));
//	usb_write(msg, 30);

}
