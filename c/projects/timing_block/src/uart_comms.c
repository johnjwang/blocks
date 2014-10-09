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
#include "uart_comms.h"
#include "io/comms.h"

#include "lcmtypes/kill_t.h"

comms_t *uart_comms = NULL;

static void uart_comms_up_int_handler(void);

void uart_comms_up_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART2);

	GPIOPinConfigure(GPIO_PG4_U2RX);
	GPIOPinConfigure(GPIO_PG5_U2TX);

	GPIOPinTypeUART(GPIO_PORTG_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPadConfigSet(GPIO_PORTG_BASE, GPIO_PIN_4, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);

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

	uart_comms = comms_create(256);
	comms_add_publisher(uart_comms, uart_comms_up_write_byte);

    // Enable the SSI peripheral interrupts.
    UARTIntRegister(UART2_BASE, uart_comms_up_int_handler);
    UARTIntEnable(UART2_BASE, UART_INT_RX | UART_INT_RT);
    IntPrioritySet(INT_UART2, 0x40);
	IntEnable(INT_UART2);
}

bool uart_comms_up_write_byte(uint8_t byte)
{
    UARTCharPut(UART2_BASE, byte);
    return true;
}

bool uart_comms_up_write(uint8_t *msg, uint32_t dataLen)
{
	uint32_t i;
	for(i = 0; i < dataLen; ++i)
	{
		uart_comms_up_write_byte(msg[i]);
	}
	return true;
}

void uart_comms_up_publish_blocking(comms_channel_t channel, uint8_t *msg, uint16_t msg_len)
{
    comms_publish_blocking(uart_comms, channel, msg, msg_len);
}

void uart_comms_up_subscribe(comms_channel_t channel, subscriber_t subscriber)
{
    comms_subscribe(uart_comms, channel, subscriber);
}

void uart_comms_up_demo(void)
{
    comms_t *uart_comms_up = comms_create(256);
    comms_add_publisher(uart_comms_up, uart_comms_up_write_byte);
	while(1)
	{
	    kill_t kill;
	    kill.reason = 1;

	    uint16_t max_len = __kill_t_encoded_array_size(&kill, 1);
	    if(max_len > 20) while(1);

        uint8_t buf[20];

        uint16_t len = __kill_t_encode_array(buf, 0, 20, &kill, 1);

	    comms_publish_blocking(uart_comms_up, CHANNEL_KILL, buf, len);

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
	    uint8_t data = UARTCharGet(UART2_BASE);
	    comms_handle(uart_comms, data);
	}
}
