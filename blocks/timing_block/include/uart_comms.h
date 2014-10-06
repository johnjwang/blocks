/*
 * uart_comms.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#ifndef UART_COMMS_H_
#define UART_COMMS_H_

void uart_comms_up_init(void);
void uart_comms_up_write(uint8_t *msg, uint32_t dataLen);
void uart_comms_up_demo(void);

#endif /* UART_COMMS_H_ */
