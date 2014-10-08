/*
 * usb_comms.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#ifndef USB_COMMS_H_
#define USB_COMMS_H_

void usb_comms_init(void);
bool usb_comms_write(uint8_t *msg, uint32_t datalen);
bool usb_comms_write_byte(uint8_t data);
void usb_comms_demo(void);

#endif /* USB_COMMS_H_ */
