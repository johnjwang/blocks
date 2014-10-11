/*
 * watchdog.c
 *
 *  Created on: Oct 10, 2014
 *      Author: Isaac Olson
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/watchdog.h"

#include "eeprom.h"
#include "watchdog.h"

static uint32_t reload_val;
static void watchdog_int_handler(void);

void watchdog_init(uint32_t tics_timeout)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);

    // Clear a watchdog reboot flag
    uint32_t eeprom_data = eeprom_read_word(EEPROM_BOOTOPT_ADDR);
    eeprom_write_word(EEPROM_BOOTOPT_ADDR, eeprom_data & ~EEPROM_BOOTOPT_WATCHDOG_MASK);

    reload_val = tics_timeout;

    WatchdogReloadSet(WATCHDOG0_BASE, reload_val);
    WatchdogResetEnable(WATCHDOG0_BASE);
    WatchdogStallEnable(WATCHDOG0_BASE);

    WatchdogIntTypeSet(WATCHDOG0_BASE, WATCHDOG_INT_TYPE_NMI);
    WatchdogIntEnable(WATCHDOG0_BASE);
    IntRegister(FAULT_NMI, watchdog_int_handler);
    IntEnable(FAULT_NMI);

    WatchdogEnable(WATCHDOG0_BASE);
    WatchdogLock(WATCHDOG0_BASE);
}

void watchdog_feed(void)
{
    WatchdogUnlock(WATCHDOG0_BASE);
    WatchdogReloadSet(WATCHDOG0_BASE, reload_val);
    WatchdogLock(WATCHDOG0_BASE);
}

static void watchdog_int_handler(void)
{
    // Purposefully do not clear the interrupt because we want the board to reset
    // WatchdogIntClear(WATCHDOG0_BASE);

    // Write a watchdog reboot flag
    uint32_t eeprom_data = eeprom_read_word(EEPROM_BOOTOPT_ADDR);
    eeprom_write_word(EEPROM_BOOTOPT_ADDR, eeprom_data | EEPROM_BOOTOPT_WATCHDOG_MASK);

    WatchdogUnlock(WATCHDOG0_BASE);
    WatchdogReloadSet(WATCHDOG0_BASE, 0); // Trigger an immediate reset of the board
    WatchdogLock(WATCHDOG0_BASE);
}
