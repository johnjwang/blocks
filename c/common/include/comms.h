/*
 * comms.h
 *
 *  Created on: Oct 5, 2014
 *      Author: jonathan
 */

#ifndef COMMS_H_
#define COMMS_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum comms_channel_t
{
    KILL,
    TELEMETRY,
    NUM_MESSAGE_TYPES

} comms_channel_t;


typedef bool (*publisher_t)(uint8_t publish_byte);
typedef void (*subscriber_t)(uint8_t msg_type, uint16_t msg_len, uint8_t *msg);

typedef struct comms_t
{
    uint32_t buf_len;
    uint8_t *buf;

    publisher_t publisher;

    subscriber_t *subscribers[NUM_MESSAGE_TYPES];
    uint8_t num_subscribers[NUM_MESSAGE_TYPES];

    uint8_t checksum1, checksum2;

} comms_t;


#endif /* COMMS_H_ */
