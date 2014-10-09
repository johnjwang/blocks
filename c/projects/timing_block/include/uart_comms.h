/*
 * uart_comms.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#ifndef UART_COMMS_H_
#define UART_COMMS_H_

#include "io/comms.h"

void uart_comms_up_init(void);
comms_t* uart_comms_up_create(int32_t buf_len);
bool uart_comms_up_write_byte(uint8_t byte);
bool uart_comms_up_write(uint8_t *msg, uint32_t dataLen);
void uart_comms_up_publish_blocking(comms_channel_t channel, uint8_t *msg, uint16_t msg_len);
void uart_comms_up_subscribe(comms_channel_t channel, subscriber_t subscriber);
void uart_comms_up_demo(void);

#endif /* UART_COMMS_H_ */
