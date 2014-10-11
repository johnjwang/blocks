/*
 * eeprom.h
 *
 *  Created on: Oct 10, 2014
 *      Author: izeke_000
 */

#ifndef EEPROM_H_
#define EEPROM_H_

// Predefined EEPROM Addresses and data masks ////////////////////////////////////////

#define EEPROM_BOOTOPT_ADDR            0x0000
#define EEPROM_BOOTOPT_DFU_MASK        0x0001 // System should boot into DFU mode
#define EEPROM_BOOTOPT_WATCHDOG_MASK   0x0002 // System was reset due to Watchdog

#define EEPROM_USB_SN_UPPER_ADDR       0x0004
#define EEPROM_USB_SN_LOWER_ADDR       0x0008

//////////////////////////////////////////////////////////////////////////////////////

void eeprom_init(void);
uint32_t eeprom_read_word(uint32_t word_addr);
void eeprom_write_word(uint32_t word_addr, uint32_t word);

#endif /* EEPROM_H_ */
