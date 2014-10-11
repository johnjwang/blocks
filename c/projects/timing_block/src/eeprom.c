/*
 * eeprom.c
 *
 *  Created on: Oct 10, 2014
 *      Author: izeke_000
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
#include "driverlib/eeprom.h"

#include "eeprom.h"

void eeprom_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);

    // XXX: should be checking the return value here and doing something intelligent
    EEPROMInit();
}

uint32_t eeprom_read_word(uint32_t word_addr)
{
    uint32_t eeprom_data[1];
    EEPROMRead(eeprom_data, word_addr, 4);
    return eeprom_data[0];
}

void eeprom_write_word(uint32_t word_addr, uint32_t word)
{
    EEPROMProgram(&word, word_addr, 4);
}
