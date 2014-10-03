/*
 * gpio_ctl.c
 *
 *  Created on: Oct 2, 2014
 *      Author: Isaac Olson
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

#include "gpio_ctl.h"

typedef struct _gpio_t
{
	uint32_t port;    // GPIO_PORTx_BASE
	uint32_t pin;     // GPIO_PIN_n

	uint32_t dir;      // GPIO_DIR_MODE_IN, GPIO_DIR_MODE_OUT, GPIO_DIR_MODE_HW
	uint32_t dr_str;   // GPIO_STRENGTH_2MA, GPIO_STRENGTH_4MA, GPIO_STRENGTH_8MA
	uint32_t pad_type; // GPIO_PIN_TYPE_STD, GPIO_PIN_TYPE_STD_WPU, GPIO_PIN_TYPE_STD_WPD

	uint32_t int_type;         // GPIO_FALLING_EDGE, GPIO_RISING_EDGE, GPIO_BOTH_EDGES
					           // GPIO_LOW_LEVEL, GPIO_HIGH_LEVEL
					           // option | with GPIO_DISCRETE_INT
	uint32_t int_flag;         // GPIO_INT_PIN_n

	uint8_t value; // 0, 1
} gpio_t;

static gpio_t gpios[NUM_GPIO] = {0};

static void gpio_t_construct();
static void gpio_int_handle_input(uint32_t input_num);
static void gpio_int_handle_portB(void);
static void gpio_int_handle_portD(void);

void gpio_ctl_init()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

	gpio_t_construct();

	int i;
	for (i=0; i<NUM_GPIO; ++i) {
		GPIODirModeSet(gpios[i].port, gpios[i].pin, gpios[i].dir);
		GPIOPadConfigSet(gpios[i].port, gpios[i].pin, gpios[i].dr_str, gpios[i].pad_type);
		if (gpios[i].int_type != 0) {
			GPIOIntTypeSet(gpios[i].port, gpios[i].pin, gpios[i].int_type);
			GPIOIntEnable(gpios[i].port, gpios[i].int_flag);
		}
	}

	for (i=GPIO_INPUT_1; i<=GPIO_INPUT_9; ++i) {
		gpios[i].value = GPIOPinRead(gpios[i].port, gpios[i].pin) != 0 ? 1 : 0;
	}
	for (i=GPIO_OUTPUT_1; i<=GPIO_OUTPUT_9; ++i) {
		GPIOPinWrite(gpios[i].port, gpios[i].pin, 0);
		gpios[i].value = 0;
	}

	IntPrioritySet(INT_GPIOB, 0x20);
	IntPrioritySet(INT_GPIOD, 0x20);

	// This function actually calls IntEnable(), so after this, interrupts can fire
	GPIOIntRegister(GPIO_PORTB_BASE, &gpio_int_handle_portB);
	GPIOIntRegister(GPIO_PORTD_BASE, &gpio_int_handle_portD);
}

static void gpio_t_construct()
{
	// Input 1
	gpios[GPIO_INPUT_1].port = GPIO_PORTD_BASE;
	gpios[GPIO_INPUT_1].pin  = GPIO_PIN_3;
	// Input 2
	gpios[GPIO_INPUT_2].port = GPIO_PORTD_BASE;
	gpios[GPIO_INPUT_2].pin  = GPIO_PIN_2;
	// Input 3
	gpios[GPIO_INPUT_3].port = GPIO_PORTD_BASE;
	gpios[GPIO_INPUT_3].pin  = GPIO_PIN_1;
	// Input 4
	gpios[GPIO_INPUT_4].port = GPIO_PORTB_BASE;
	gpios[GPIO_INPUT_4].pin  = GPIO_PIN_6;
	// Input 5
	gpios[GPIO_INPUT_5].port = GPIO_PORTB_BASE;
	gpios[GPIO_INPUT_5].pin  = GPIO_PIN_4;
	// Input 6
	gpios[GPIO_INPUT_6].port = GPIO_PORTB_BASE;
	gpios[GPIO_INPUT_6].pin  = GPIO_PIN_5;
	// Input 7
	gpios[GPIO_INPUT_7].port = GPIO_PORTD_BASE;
	gpios[GPIO_INPUT_7].pin  = GPIO_PIN_6;
	// Input 8
	gpios[GPIO_INPUT_8].port = GPIO_PORTB_BASE;
	gpios[GPIO_INPUT_8].pin  = GPIO_PIN_3;
	// Input 9
	gpios[GPIO_INPUT_9].port = GPIO_PORTB_BASE;
	gpios[GPIO_INPUT_9].pin  = GPIO_PIN_2;

	// Output 1
	gpios[GPIO_OUTPUT_1].port = GPIO_PORTG_BASE;
	gpios[GPIO_OUTPUT_1].pin  = GPIO_PIN_1;
	// Output 2
	gpios[GPIO_OUTPUT_2].port = GPIO_PORTG_BASE;
	gpios[GPIO_OUTPUT_2].pin  = GPIO_PIN_2;
	// Output 3
	gpios[GPIO_OUTPUT_3].port = GPIO_PORTG_BASE;
	gpios[GPIO_OUTPUT_3].pin  = GPIO_PIN_3;
	// Output 4
	gpios[GPIO_OUTPUT_4].port = GPIO_PORTC_BASE;
	gpios[GPIO_OUTPUT_4].pin  = GPIO_PIN_4;
	// Output 5
	gpios[GPIO_OUTPUT_5].port = GPIO_PORTC_BASE;
	gpios[GPIO_OUTPUT_5].pin  = GPIO_PIN_5;
	// Output 6
	gpios[GPIO_OUTPUT_6].port = GPIO_PORTC_BASE;
	gpios[GPIO_OUTPUT_6].pin  = GPIO_PIN_6;
	// Output 7
	gpios[GPIO_OUTPUT_7].port = GPIO_PORTC_BASE;
	gpios[GPIO_OUTPUT_7].pin  = GPIO_PIN_7;
	// Output 8
	gpios[GPIO_OUTPUT_8].port = GPIO_PORTB_BASE;
	gpios[GPIO_OUTPUT_8].pin  = GPIO_PIN_7;
	// Output 9
	gpios[GPIO_OUTPUT_9].port = GPIO_PORTG_BASE;
	gpios[GPIO_OUTPUT_9].pin  = GPIO_PIN_0;

	int i;

	// Generic Inputs
	for (i=GPIO_INPUT_1; i<=GPIO_INPUT_9; ++i) {
		gpios[i].dir      = GPIO_DIR_MODE_IN;
		gpios[i].dr_str   = GPIO_STRENGTH_4MA;
		gpios[i].pad_type = GPIO_PIN_TYPE_STD_WPU;
		gpios[i].int_type = GPIO_BOTH_EDGES;

		switch (gpios[i].pin) {
			case GPIO_PIN_0:
				gpios[i].int_flag = GPIO_INT_PIN_0;
				break;
			case GPIO_PIN_1:
				gpios[i].int_flag = GPIO_INT_PIN_1;
				break;
			case GPIO_PIN_2:
				gpios[i].int_flag = GPIO_INT_PIN_2;
				break;
			case GPIO_PIN_3:
				gpios[i].int_flag = GPIO_INT_PIN_3;
				break;
			case GPIO_PIN_4:
				gpios[i].int_flag = GPIO_INT_PIN_4;
				break;
			case GPIO_PIN_5:
				gpios[i].int_flag = GPIO_INT_PIN_5;
				break;
			case GPIO_PIN_6:
				gpios[i].int_flag = GPIO_INT_PIN_6;
				break;
			case GPIO_PIN_7:
				gpios[i].int_flag = GPIO_INT_PIN_7;
				break;
			default:
				break;
		}
	}

	// Generic Outputs
	for (i=GPIO_OUTPUT_1; i<=GPIO_OUTPUT_9; ++i) {
		gpios[i].dir      = GPIO_DIR_MODE_OUT;
		gpios[i].dr_str   = GPIO_STRENGTH_4MA;
		gpios[i].pad_type = GPIO_PIN_TYPE_STD_WPU;
		// int_type, flag, and handler are left as 0
	}
}

int8_t gpio_ctl_read(uint32_t gpio_num)
{
	if (gpio_num >= NUM_GPIO) return -1;
	return GPIOPinRead(gpios[gpio_num].port, gpios[gpio_num].pin) ? 1 : 0;
}

void gpio_ctl_write(uint32_t gpio_num, uint8_t value)
{
	if (gpio_num >= NUM_GPIO) return;

	if (gpios[gpio_num].dir == GPIO_DIR_MODE_IN) {
		gpios[gpio_num].pad_type = value != 0 ? GPIO_PIN_TYPE_STD_WPU : GPIO_PIN_TYPE_STD_WPD;
		GPIOPadConfigSet(gpios[gpio_num].port, gpios[gpio_num].pin, gpios[gpio_num].dr_str, gpios[gpio_num].pad_type);
	} else {
		value != 0 ? GPIOPinWrite(gpios[gpio_num].port, gpios[gpio_num].pin,  gpios[gpio_num].pin)
				   : GPIOPinWrite(gpios[gpio_num].port, gpios[gpio_num].pin, ~gpios[gpio_num].pin);
		gpios[gpio_num].value = value != 0 ? 1 : 0;
	}
}

int32_t gpio_ctl_values_snprintf(uint8_t *buf, uint32_t len)
{
	int i;
	for (i=GPIO_INPUT_1; i<=GPIO_OUTPUT_9; ++i) {
		gpios[i].value = gpio_ctl_read(i);
	}

	return snprintf((char*)buf, len, "%hu%hu%hu%hu%hu%hu%hu%hu%hu %hu%hu%hu%hu%hu%hu%hu%hu%hu\r\n",
			gpios[GPIO_INPUT_1].value, gpios[GPIO_INPUT_2].value, gpios[GPIO_INPUT_3].value,
			gpios[GPIO_INPUT_4].value, gpios[GPIO_INPUT_5].value, gpios[GPIO_INPUT_6].value,
			gpios[GPIO_INPUT_7].value, gpios[GPIO_INPUT_8].value, gpios[GPIO_INPUT_9].value,
			gpios[GPIO_OUTPUT_1].value, gpios[GPIO_OUTPUT_2].value, gpios[GPIO_OUTPUT_3].value,
			gpios[GPIO_OUTPUT_4].value, gpios[GPIO_OUTPUT_5].value, gpios[GPIO_OUTPUT_6].value,
			gpios[GPIO_OUTPUT_7].value, gpios[GPIO_OUTPUT_8].value, gpios[GPIO_OUTPUT_9].value);
}

int32_t gpio_ctl_values_snprintf_no_preread(uint8_t *buf, uint32_t len)
{
	return snprintf((char*)buf, len, "%hu%hu%hu%hu%hu%hu%hu%hu%hu %hu%hu%hu%hu%hu%hu%hu%hu%hu\r\n",
			gpios[GPIO_INPUT_1].value, gpios[GPIO_INPUT_2].value, gpios[GPIO_INPUT_3].value,
			gpios[GPIO_INPUT_4].value, gpios[GPIO_INPUT_5].value, gpios[GPIO_INPUT_6].value,
			gpios[GPIO_INPUT_7].value, gpios[GPIO_INPUT_8].value, gpios[GPIO_INPUT_9].value,
			gpios[GPIO_OUTPUT_1].value, gpios[GPIO_OUTPUT_2].value, gpios[GPIO_OUTPUT_3].value,
			gpios[GPIO_OUTPUT_4].value, gpios[GPIO_OUTPUT_5].value, gpios[GPIO_OUTPUT_6].value,
			gpios[GPIO_OUTPUT_7].value, gpios[GPIO_OUTPUT_8].value, gpios[GPIO_OUTPUT_9].value);
}

static void gpio_int_handle_input(uint32_t input_num)
{
	gpios[input_num].value =
			GPIOPinRead(gpios[input_num].port, gpios[input_num].pin) != 0 ? 1 : 0;
}

static void gpio_int_handle_portB(void)
{
	int32_t intStatus = GPIOIntStatus(GPIO_PORTB_BASE, false);

	GPIOIntClear(GPIO_PORTB_BASE, intStatus);

	if (intStatus & GPIO_INT_PIN_6) gpio_int_handle_input(GPIO_INPUT_4);
	if (intStatus & GPIO_INT_PIN_4) gpio_int_handle_input(GPIO_INPUT_5);
	if (intStatus & GPIO_INT_PIN_5) gpio_int_handle_input(GPIO_INPUT_6);
	if (intStatus & GPIO_INT_PIN_3) gpio_int_handle_input(GPIO_INPUT_8);
	if (intStatus & GPIO_INT_PIN_2) gpio_int_handle_input(GPIO_INPUT_9);
}

static void gpio_int_handle_portD(void)
{
	int32_t intStatus = GPIOIntStatus(GPIO_PORTD_BASE, false);

	GPIOIntClear(GPIO_PORTD_BASE, intStatus);

	if (intStatus & GPIO_INT_PIN_3) gpio_int_handle_input(GPIO_INPUT_1);
	if (intStatus & GPIO_INT_PIN_2)	gpio_int_handle_input(GPIO_INPUT_2);
	if (intStatus & GPIO_INT_PIN_1) gpio_int_handle_input(GPIO_INPUT_3);
	if (intStatus & GPIO_INT_PIN_6) gpio_int_handle_input(GPIO_INPUT_7);
}
