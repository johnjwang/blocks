/*
 * usb_comms.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#ifndef USB_COMMS_H_
#define USB_COMMS_H_

#include "io/comms.h"

extern comms_t *usb_comms;

void usb_comms_init(uint32_t max_num_tx_origins);
bool usb_comms_write(uint8_t *msg, uint16_t datalen);
bool usb_comms_write_byte(uint8_t data);
void usb_comms_publish(comms_channel_t channel, uint8_t *msg, uint16_t msg_len);
void usb_comms_publish_id(uint16_t id, comms_channel_t channel,
                          uint8_t *msg, uint32_t tx_origin_num, uint16_t msg_len);
comms_status_t usb_comms_transmit(void);
void usb_comms_subscribe(comms_channel_t channel, subscriber_t subscriber, void *usr);
void usb_comms_demo(void);

#endif /* USB_COMMS_H_ */
