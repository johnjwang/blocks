/*
 * i2c_comms.h
 *
 *  Created on: Oct 13, 2014
 *      Author: Jonathan
 */

#ifndef I2C_COMMS_H_
#define I2C_COMMS_H_

#include <stdint.h>
#include <stdbool.h>

#include "io/comms.h"

extern comms_t *i2c_comms;

void i2c_comms_init(void);
bool i2c_comms_write_byte(uint8_t byte);
bool i2c_comms_write(uint8_t *msg, uint16_t dataLen);
void i2c_comms_publish(comms_channel_t channel, uint8_t *msg, uint16_t msg_len);
void i2c_comms_subscribe(comms_channel_t channel, subscriber_t subscriber, void *usr);
void i2c_comms_demo(void);

#endif /* I2C_COMMS_H_ */
