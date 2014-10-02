/*
 * bootloader.c
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_nvic.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/systick.h"
#include "driverlib/eeprom.h"
#include "driverlib/interrupt.h"

#include "timing_block_util.h"
#include "bootloader.h"

#define ROM_UpdateUSB\
        ((void (*)(uint8_t *pui8DescriptorInfo))ROM_USBTABLE[58])

static void usb_load_new();


void bootloader_check_upload()
{
    uint32_t eeprom_data[1];
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    EEPROMInit();

    // Read first word from eeprom
    EEPROMRead(eeprom_data, 0, 4);
    uint32_t reprogram_usb = eeprom_data[0];

    // Write a 0 to first word in eeprom
    eeprom_data[0] = 0x00;
    EEPROMProgram(eeprom_data, 0, 4);

    // If the word we read was a 1, that means someone
    // pushed the reset button and put the board in dfu mode
    if(reprogram_usb == 1)
    	usb_load_new();

    // Write a 1 to the first word of eeprom
    eeprom_data[0] = 0x01;
    EEPROMProgram(eeprom_data, 0, 4);

    // blink the leds 3 times to indicate window in which
    // pressing reset button would put device into dfu mode
	blink_led(1,3);
	blink_led(2,3);
	blink_led(3,3);
	blink_led(4,3);
	SysCtlDelay(30000000);

	// Write a 0 back to the first word of eeprom
	eeprom_data[0] = 0x00;
	EEPROMProgram(eeprom_data, 0, 4);

}

static void usb_load_new()
{
	// Configure the required pins for USB operation.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

	// Disable all interrupts
	IntMasterDisable();
	SysTickIntDisable();
	SysTickDisable();
	HWREG(NVIC_DIS0) = 0xffffffff;
	HWREG(NVIC_DIS1) = 0xffffffff;

	// 1. Enable USB PLL
	// 2. Enable USB controller
	SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_USB0);
	SysCtlUSBPLLEnable();

	// 3. Enable USB D+ D- pins

	// 4. Activate USB DFU
	SysCtlDelay(SysCtlClockGet() / 3);
	IntMasterEnable(); // Re-enable interrupts at NVIC level
	ROM_UpdateUSB(0);
	// 5. Should never get here since update is in progress
}


