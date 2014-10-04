/*
 * usb_comms.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#ifndef USB_COMMS_H_
#define USB_COMMS_H_

void usb_init(void);
bool usb_write(uint8_t *msg, uint32_t datalen);
bool usb_write_char(char data);
void usb_demo(void);

#endif /* USB_COMMS_H_ */
