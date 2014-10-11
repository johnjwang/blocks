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
#include "eeprom.h"
#include "bootloader.h"

// XXX add another bit to EEPROM that is "ignore bootload" to make boots fast if you have to
//     restart due to watchdogs

#define ROM_UpdateUSB\
        ((void (*)(uint8_t *pui8DescriptorInfo))ROM_USBTABLE[58])

static void usb_load_new();

void bootloader_check_upload()
{
    // Check for DFU bit and watchdog bit
    uint32_t eeprom_data = eeprom_read_word(EEPROM_BOOTOPT_ADDR);
    uint8_t reprogram_usb = eeprom_data & EEPROM_BOOTOPT_DFU_MASK;
    uint8_t skip_bootloader = eeprom_data & EEPROM_BOOTOPT_WATCHDOG_MASK;

    // Disable DFU bit for next boot
    eeprom_write_word(EEPROM_BOOTOPT_ADDR, eeprom_data & ~EEPROM_BOOTOPT_DFU_MASK);

    // Skip bootload if booted from watchdog
    if (skip_bootloader) {
        turn_on_led(2);
        return;
    }

    // If the word we read was a 1, that means someone
    // pushed the reset button and put the board in dfu mode
    if(reprogram_usb != 0) usb_load_new();

    // Prep EEPROM with DFU signal in case of reset
    eeprom_write_word(EEPROM_BOOTOPT_ADDR, eeprom_data | EEPROM_BOOTOPT_DFU_MASK);

    // blink the leds 3 times to indicate window in which
    // pressing reset button would put device into dfu mode
	uint8_t i;
    for(i = 0; i < 3; i++)
	{
        turn_on_led(1);
        turn_on_led(2);
        turn_on_led(3);
        turn_on_led(4);
        SysCtlDelay(6000000);
        turn_off_led(1);
        turn_off_led(2);
        turn_off_led(3);
        turn_off_led(4);
        SysCtlDelay(4000000);
	}
    SysCtlDelay(10000000);

	// Wipe the DFU prep bit from the EEPROM to prevent DFU on next boot
	eeprom_write_word(EEPROM_BOOTOPT_ADDR, eeprom_data & ~EEPROM_BOOTOPT_DFU_MASK);

}

static void usb_load_new()
{
    turn_on_led(1); turn_on_led(2); turn_on_led(3); turn_on_led(4);
	// Configure the required pins for USB operation.
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIOPinTypeUSBAnalog(GPIO_PORTD_BASE, GPIO_PIN_5 | GPIO_PIN_4);

	// Disable all interrupts
	IntMasterDisable();
	SysTickIntDisable();
	SysTickDisable();
	HWREG(NVIC_DIS0) = 0xffffffff;
	HWREG(NVIC_DIS1) = 0xffffffff;
    HWREG(NVIC_DIS2) = 0xffffffff;
    HWREG(NVIC_DIS3) = 0xffffffff;
    HWREG(NVIC_DIS4) = 0xffffffff;

	// 1. Enable USB PLL
	// 2. Enable USB controller
	SysCtlPeripheralEnable(SYSCTL_PERIPH_USB0);
	SysCtlPeripheralReset(SYSCTL_PERIPH_USB0);
	SysCtlUSBPLLEnable();

	// 3. Enable USB D+ D- pins

	// 4. Activate USB DFU
	SysCtlDelay(SysCtlClockGet() / 3);

	turn_off_led(2);
	turn_off_led(3);
	turn_off_led(4);

	IntMasterEnable(); // Re-enable interrupts at NVIC level
	ROM_UpdateUSB(0);
	// 5. Should never get here since update is in progress

}


