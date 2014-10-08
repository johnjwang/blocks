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
    CHANNEL_KILL,
    CHANNEL_TELEMETRY,
    CHANNEL_NUM_CHANNELS

} comms_channel_t;


typedef bool (*publisher_t)(uint8_t publish_byte);
typedef void (*subscriber_t)(uint8_t *msg, uint16_t msg_len);

typedef struct comms_t
{
    publisher_t publisher;

    uint8_t *decode_buf;
    uint32_t decode_buf_len;

    subscriber_t *subscribers[CHANNEL_NUM_CHANNELS];
    uint8_t num_subscribers[CHANNEL_NUM_CHANNELS];

    // Variables to handle decoding
    comms_channel_t decode_channel;
    uint16_t decode_data_len;
    uint16_t decode_num_data_read;
    uint16_t decode_checksum;
    uint8_t  decode_state;
    uint8_t checksum_rx1, checksum_rx2;
    uint8_t checksum_tx1, checksum_tx2;

} comms_t;

comms_t* comms_create(publisher_t publisher, int32_t buf_len);

void comms_subscribe(comms_t *comms, comms_channel_t channel, subscriber_t subscriber);

void comms_publish_blocking(comms_t *comms,
                            comms_channel_t channel,
                            uint8_t *msg,
                            uint16_t msg_len);

void comms_handle(comms_t *comms, uint8_t byte);

void comms_destroy(comms_t *comms);

#endif /* COMMS_H_ */
