/*
 * uart_comms.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#ifndef UART_COMMS_H_
#define UART_COMMS_H_

#include "io/comms.h"

void uart_up_comms_init(void);
comms_t* uart_up_comms_create(int32_t buf_len);
bool uart_up_comms_write_byte(uint8_t byte);
bool uart_up_comms_write(uint8_t *msg, uint16_t dataLen);
void uart_up_comms_publish_blocking(comms_channel_t channel, uint8_t *msg, uint16_t msg_len);
void uart_up_comms_subscribe(comms_channel_t channel, subscriber_t subscriber, void *usr);
void uart_up_comms_demo(void);

#endif /* UART_COMMS_H_ */
