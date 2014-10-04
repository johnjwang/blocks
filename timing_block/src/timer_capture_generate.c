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
uint32_t ms_per_capture_period = 60;
uint32_t ms_per_pwm_period = 20;
uint32_t cycles_per_capture_period;
uint32_t cycles_per_pwm_period;
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
	GPIOPinTypeTimer(GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3                                                    );


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
	cycles_per_capture_period = (ms_per_capture_period * sys_clk_cycles_per_second) / 1000;
	cycles_per_pwm_period     = (ms_per_pwm_period * sys_clk_cycles_per_second / 1000);

	// Set Prescale Value
	uint32_t pwm_prescaler     = (cycles_per_pwm_period     & 0xFF0000) >> 16;
	uint32_t capture_prescaler = (cycles_per_capture_period & 0xFF0000) >> 16;
    TimerPrescaleSet(TIMER0_BASE,  TIMER_A, capture_prescaler); // input
    TimerPrescaleSet(TIMER0_BASE,  TIMER_B, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER1_BASE,  TIMER_A, capture_prescaler); // input
    TimerPrescaleSet(TIMER1_BASE,  TIMER_B, capture_prescaler); // input
    TimerPrescaleSet(TIMER3_BASE,  TIMER_A, capture_prescaler); // input
    TimerPrescaleSet(TIMER3_BASE,  TIMER_B, capture_prescaler); // input
    TimerPrescaleSet(TIMER4_BASE,  TIMER_A, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER4_BASE,  TIMER_B, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER5_BASE,  TIMER_A, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER5_BASE,  TIMER_B, pwm_prescaler      ); // output
//    TimerPrescaleSet(WTIMER0_BASE, TIMER_A, 0                  ); // output
//    TimerPrescaleSet(WTIMER0_BASE, TIMER_B, 0                  ); // output
//    TimerPrescaleSet(WTIMER1_BASE, TIMER_A, 0                  ); // output
//    TimerPrescaleSet(WTIMER1_BASE, TIMER_B, 0                  ); // output
//    TimerPrescaleSet(WTIMER2_BASE, TIMER_B, 0                  ); // input
//    TimerPrescaleSet(WTIMER3_BASE, TIMER_A, 0                  ); // input
//    TimerPrescaleSet(WTIMER3_BASE, TIMER_B, 0                  ); // input
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_A, 0                  ); // input

	// Set Load Value
    uint32_t pwm_load     = cycles_per_pwm_period     & 0xFFFF;
    uint32_t capture_load = cycles_per_capture_period & 0xFFFF;
	TimerLoadSet(TIMER0_BASE,  TIMER_A, capture_load);
	TimerLoadSet(TIMER0_BASE,  TIMER_B, pwm_load                  );
	TimerLoadSet(TIMER1_BASE,  TIMER_A, capture_load              );
	TimerLoadSet(TIMER1_BASE,  TIMER_B, capture_load              );
	TimerLoadSet(TIMER3_BASE,  TIMER_A, capture_load              );
	TimerLoadSet(TIMER3_BASE,  TIMER_B, capture_load              );
	TimerLoadSet(TIMER4_BASE,  TIMER_A, pwm_load                  );
	TimerLoadSet(TIMER4_BASE,  TIMER_B, pwm_load                  );
	TimerLoadSet(TIMER5_BASE,  TIMER_A, pwm_load                  );
	TimerLoadSet(TIMER5_BASE,  TIMER_B, pwm_load                  );
  	TimerLoadSet(WTIMER0_BASE, TIMER_A, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER0_BASE, TIMER_B, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER1_BASE, TIMER_A, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER1_BASE, TIMER_B, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER2_BASE, TIMER_B, cycles_per_capture_period );
  	TimerLoadSet(WTIMER3_BASE, TIMER_A, cycles_per_capture_period );
  	TimerLoadSet(WTIMER3_BASE, TIMER_B, cycles_per_capture_period );
  	TimerLoadSet(WTIMER5_BASE, TIMER_A, cycles_per_capture_period );

	// Set PWM Generators to Update Match Value After Overflow
	TimerUpdateMode(TIMER0_BASE,  TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER4_BASE,  TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER4_BASE,  TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER5_BASE,  TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER5_BASE,  TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER0_BASE, TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER0_BASE, TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER1_BASE, TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER1_BASE, TIMER_B, TIMER_UP_MATCH_TIMEOUT);

    // Set PWM to 0 Duty Cycle
	uint32_t duty_cycle = cycles_per_pwm_period / 10; // 10% duty cycle
	timer_generate_pulse(duty_cycle);

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
}

void timer_capture_generate_start(void)
{
    // Enable Timer
    TimerEnable(TIMER0_BASE,  TIMER_B);
    TimerEnable(TIMER4_BASE,  TIMER_A);
    TimerEnable(TIMER4_BASE,  TIMER_B);
    TimerEnable(TIMER5_BASE,  TIMER_A);
    TimerEnable(TIMER5_BASE,  TIMER_B);
    TimerEnable(WTIMER0_BASE, TIMER_A);
    TimerEnable(WTIMER0_BASE, TIMER_B);
    TimerEnable(WTIMER1_BASE, TIMER_A);
    TimerEnable(WTIMER1_BASE, TIMER_B);
}

void timer_generate_pulse_percent(float percent)
{
    timer_generate_pulse(percent * cycles_per_pwm_period);
}

void timer_generate_pulse(uint32_t pulse_width)
{
	uint32_t match_set;
	if (pulse_width > cycles_per_pwm_period) match_set = 0;
	else if(pulse_width == 0) match_set = cycles_per_pwm_period - 1;
	else match_set = cycles_per_pwm_period - pulse_width;

    uint32_t match_set_upper_8  = (match_set & 0xff0000) >> 16;
    uint32_t match_set_lower_16 =  match_set & 0xffff;
    TimerPrescaleMatchSet(TIMER0_BASE,  TIMER_B, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER4_BASE,  TIMER_A, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER4_BASE,  TIMER_B, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER5_BASE,  TIMER_A, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER5_BASE,  TIMER_B, match_set_upper_8);
    TimerMatchSet(TIMER0_BASE,  TIMER_B, match_set_lower_16);
    TimerMatchSet(TIMER4_BASE,  TIMER_A, match_set_lower_16);
    TimerMatchSet(TIMER4_BASE,  TIMER_B, match_set_lower_16);
    TimerMatchSet(TIMER5_BASE,  TIMER_A, match_set_lower_16);
    TimerMatchSet(TIMER5_BASE,  TIMER_B, match_set_lower_16);
    TimerMatchSet(WTIMER0_BASE, TIMER_A, match_set);
    TimerMatchSet(WTIMER0_BASE, TIMER_B, match_set);
    TimerMatchSet(WTIMER1_BASE, TIMER_A, match_set);
    TimerMatchSet(WTIMER1_BASE, TIMER_B, match_set);

    uint8_t msg[20] = {0};
    usb_write(msg, snprintf((char*)msg, 20, "%lu\r\n", match_set));
}

static void timer_capture_int_handler(void)
{
	TimerIntClear(TIMER1_BASE, TIMER_CAPA_EVENT);

	static uint8_t msg[30];

	if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_4) == 0) {
		usb_write(CAPTURE_LOW_MSG, CAPTURE_LOW_MSG_LEN);
	} else {
		usb_write(CAPTURE_HIGH_MSG, CAPTURE_HIGH_MSG_LEN);
	}
	snprintf((char*)msg, 30, "%lu\r\n", TimerValueGet(TIMER1_BASE, TIMER_A));
	usb_write(msg, 30);

}
