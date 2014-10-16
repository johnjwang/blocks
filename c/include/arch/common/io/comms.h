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
    CHANNEL_CFG_USB_SN,
    CHANNEL_CFG_DATA_FREQUENCY,
    CHANNEL_DEBUG,
    CHANNEL_NUM_CHANNELS

} comms_channel_t;

typedef struct comms_t comms_t;

typedef bool (*publisher_t)(uint8_t *data, uint16_t data_len);

typedef void (*subscriber_t)(void *usr, uint16_t id, comms_channel_t channel,
                             uint8_t *msg, uint16_t msg_len);




comms_t* comms_create(uint32_t buf_len_rx, uint32_t buf_len_tx);

void comms_add_publisher(comms_t *comms, publisher_t publisher);

void comms_subscribe(comms_t *comms, comms_channel_t channel,
                     subscriber_t subscriber, void *usr);

inline void comms_publish(comms_t *comms,
                          comms_channel_t channel,
                          uint8_t *msg,
                          uint16_t msg_len);

void comms_publish_id(comms_t *comms,
                      uint16_t id,
                      comms_channel_t channel,
                      uint8_t *msg,
                      uint16_t msg_len);

void comms_handle(comms_t *comms, uint8_t byte);

void comms_destroy(comms_t *comms);

#endif /* COMMS_H_ */
