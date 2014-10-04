/*
 * uart_comms.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */
#include<stdint.h>
#include<stdbool.h>
#include<string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

#include "timer_capture_generate.h"
#include "usb_comms.h"

static void uart_comms_up_int_handler(void);

void uart_comms_up_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

	GPIOPinConfigure(GPIO_PG4_U2RX);
	GPIOPinConfigure(GPIO_PG5_U2TX);

	GPIOPinTypeUART(GPIO_PORTG_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    UARTConfigSetExpClk(UART2_BASE, SysCtlClockGet(), 9600,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                         UART_CONFIG_PAR_NONE));

	UARTEnable(UART2_BASE);

	//
	// Read any residual data from the UART port.  This makes sure the receive
	// FIFOs are empty, so we don't read any unwanted junk. The
	// UARTDataGetNonBlocking function returns "true" when data was returned,
	// and "false" when no data was returned. The "non-blocking" function checks
	// if there is any data in the receive FIFO and does not "hang" if there isn't.
	//
	while(UARTCharsAvail(UART2_BASE))
	{
		UARTCharGetNonBlocking(UART2_BASE);
	}

    // Enable the SSI peripheral interrupts.
    UARTIntRegister(UART2_BASE, uart_comms_up_int_handler);
    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT);
    IntPrioritySet(INT_UART2, 0x40);
	IntEnable(INT_UART2);
}

void uart_comms_up_write(uint8_t *msg, uint32_t dataLen)
{
	uint32_t i;
	for(i = 0; i < dataLen; ++i)
	{
		UARTCharPut(UART2_BASE, msg[i]);
	}
}

void uart_comms_up_demo(void)
{
	while(1)
	{
		static uint8_t *msg = "This is a test\r\n";
		uart_comms_up_write(msg, strlen((char*)msg));
		SysCtlDelay(SysCtlClockGet() / 3);
	}
}

static void uart_comms_up_int_handler(void)
{
    //
    // Clear the timer interrupt.
    //
	uint32_t mode = UARTIntStatus(UART2_BASE, true);
    UARTIntClear(UART2_BASE, mode);

	while(UARTCharsAvail(UART2_BASE))
	{
	    uint32_t data = UARTCharGet(UART2_BASE);
	    usb_write_char((char)data);
	    timer_generate_pulse_percent(((char)data - '0') / 10.0);
	}
}
