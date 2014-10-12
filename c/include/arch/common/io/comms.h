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
    CHANNEL_ALL,
    CHANNEL_KILL,
    CHANNEL_CHANNELS,
    CHANNEL_TELEMETRY,
    CHANNEL_USB_SN,
    CHANNEL_DEBUG,
    CHANNEL_NUM_CHANNELS

} comms_channel_t;

typedef struct comms_t comms_t;

typedef bool (*publisher_t)(uint8_t publish_byte);

typedef void (*subscriber_t)(void *usr, uint16_t id, comms_channel_t channel,
                             uint8_t *msg, uint16_t msg_len);




comms_t* comms_create(int32_t buf_len);

void comms_add_publisher(comms_t *comms, publisher_t publisher);

void comms_subscribe(comms_t *comms, comms_channel_t channel, subscriber_t subscriber, void *usr);

void comms_publish_blocking_id(comms_t *comms,
                               uint16_t id,
                               comms_channel_t channel,
                               uint8_t *msg,
                               uint16_t msg_len);

inline void comms_publish_blocking(comms_t *comms,
                                   comms_channel_t channel,
                                   uint8_t *msg,
                                   uint16_t msg_len);

void comms_handle(comms_t *comms, uint8_t byte);

void comms_destroy(comms_t *comms);

#endif /* COMMS_H_ */
