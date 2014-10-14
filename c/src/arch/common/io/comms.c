/*
 * comms.c
 *
 *  Created on: Oct 5, 2014
 *      Author: Jonathan
 */
#include <stdlib.h>
#include <stdint.h>

#include "io/comms.h"

typedef struct subscriber_container_t
{
    subscriber_t subs;
    void *usr;

} subscriber_container_t;


struct comms_t
{
    uint8_t *buf_rx;
    uint32_t buf_len_rx;

    uint8_t *buf_tx;
    uint32_t buf_len_tx;

    publisher_t *publishers; // An array of publisher function pointers
    uint8_t num_publishers;

    // An array of pointers to arrays of
    // subscriber containers
    subscriber_container_t **subscribers[CHANNEL_NUM_CHANNELS];
    uint8_t num_subscribers[CHANNEL_NUM_CHANNELS];

    // Variables to handle decoding
    comms_channel_t decode_channel;
    uint16_t decode_data_len;
    uint16_t decode_num_data_read;
    uint16_t decode_checksum;
    uint16_t decode_id;
    uint8_t  decode_state;
    uint8_t  checksum_rx1, checksum_rx2;
    uint8_t  checksum_tx1, checksum_tx2;

    // Variables to handle encoding
    uint16_t encode_data_len;
};


#define START_BYTE1 0xB1
#define START_BYTE2 0x75

static void fletcher_checksum_clear_rx(comms_t *comms);
static void fletcher_checksum_add_byte_rx(comms_t *comms, uint8_t byte);
static uint16_t fletcher_checksum_calculate_rx(comms_t *comms);
static void fletcher_checksum_clear_tx(comms_t *comms);
static void fletcher_checksum_add_byte_tx(comms_t *comms, uint8_t byte);
static uint16_t fletcher_checksum_calculate_tx(comms_t *comms);
static inline uint16_t fletcher_checksum_calculate(uint8_t checksum1, uint8_t checksum2);

comms_t* comms_create(uint32_t buf_len_rx, uint32_t buf_len_tx)
{
    comms_t *ret = (comms_t*) malloc(sizeof(comms_t));
    if(ret == NULL) return NULL;

    ret->num_publishers = 0;
    ret->publishers = NULL;

    ret->buf_len_rx = buf_len_rx;
    ret->buf_rx = (uint8_t*) malloc(sizeof(uint8_t) * buf_len_rx);

    // This makes our lives easier when handling
    // the buffering of messages to be published
    if(buf_len_tx == 0) buf_len_tx = 1;
    ret->buf_len_tx = buf_len_tx;
    ret->buf_tx = (uint8_t*) malloc(sizeof(uint8_t) * buf_len_tx);

    ret->decode_state = 0;
    ret->decode_id = 0;
    ret->decode_channel = (comms_channel_t) 0;
    ret->decode_data_len = 0;
    ret->encode_data_len = 0;
    fletcher_checksum_clear_rx(ret);
    fletcher_checksum_clear_tx(ret);

    uint8_t i;
    for(i = 0; i < CHANNEL_NUM_CHANNELS; ++i)
    {
        ret->subscribers[i] = NULL;
        ret->num_subscribers[i] = 0;
    }

    return ret;
}

void comms_add_publisher(comms_t *comms,
                                  publisher_t publisher)
{
    comms->num_publishers++;
    comms->publishers = (publisher_t*) realloc(comms->publishers,
                                                        sizeof(publisher_t) *
                                                        comms->num_publishers);
    comms->publishers[comms->num_publishers - 1] = publisher;
}

void comms_subscribe(comms_t *comms,
                     comms_channel_t channel,
                     subscriber_t subscriber,
                     void *usr)
{
    if(channel >= CHANNEL_NUM_CHANNELS) while(1);
    comms->num_subscribers[channel]++;
    comms->subscribers[channel] = (subscriber_container_t**) realloc(comms->subscribers[channel],
                                                                     comms->num_subscribers[channel] *
                                                                     sizeof(subscriber_container_t*));
    comms->subscribers[channel][comms->num_subscribers[channel] - 1] =
        (subscriber_container_t*) malloc(sizeof(subscriber_container_t));
    comms->subscribers[channel][comms->num_subscribers[channel] - 1]->subs = subscriber;
    comms->subscribers[channel][comms->num_subscribers[channel] - 1]->usr  = usr;
}

static void publish_flush(comms_t *comms)
{
    uint8_t i;
    for(i = 0; i < comms->num_publishers; ++i)
        comms->publishers[i](comms->buf_tx, comms->encode_data_len);

    comms->encode_data_len = 0;
}

static void publish(comms_t *comms, uint8_t data)
{
    comms->buf_tx[comms->encode_data_len++] = data;

    if(comms->encode_data_len == comms->buf_len_tx)
        publish_flush(comms);
}

inline void comms_publish(comms_t *comms,
                                   comms_channel_t channel,
                                   uint8_t *msg,
                                   uint16_t msg_len)
{
    comms_publish_id(comms, 0, channel, msg, msg_len);
}

void comms_publish_id(comms_t *comms,
                               uint16_t id,
                               comms_channel_t channel,
                               uint8_t *msg,
                               uint16_t msg_len)
{
#define NUM_METADATA 1+1+2+1+2+2

    fletcher_checksum_clear_tx(comms);

    fletcher_checksum_add_byte_tx(comms, START_BYTE1);
    publish(comms, START_BYTE1);

    fletcher_checksum_add_byte_tx(comms, START_BYTE2);
    publish(comms, START_BYTE2);

    uint8_t id1 = (id & 0xff00) >> 8;
    fletcher_checksum_add_byte_tx(comms, id1);
    publish(comms, id1);

    uint8_t id2 = id & 0x00ff;
    fletcher_checksum_add_byte_tx(comms, id2);
    publish(comms, id2);

    fletcher_checksum_add_byte_tx(comms, channel);
    publish(comms, channel);

    uint8_t len1 = (msg_len & 0xff00) >> 8;
    fletcher_checksum_add_byte_tx(comms, len1);
    publish(comms, len1);

    uint8_t len2 = msg_len & 0x00ff;
    fletcher_checksum_add_byte_tx(comms, len2);
    publish(comms, len2);

    uint8_t i;
    for(i = 0; i< msg_len; ++i)
    {
        fletcher_checksum_add_byte_tx(comms, msg[i]);
        publish(comms, msg[i]);
    }

    fletcher_checksum_calculate_tx(comms);

    publish(comms, comms->checksum_tx1);
    publish(comms, comms->checksum_tx2);

    publish_flush(comms);

#undef NUM_METADATA
}

void comms_handle(comms_t *comms, uint8_t byte)
{
#define STATE_START1    0
#define STATE_START2    1
#define STATE_ID1       2
#define STATE_ID2       3
#define STATE_CHANNEL   4
#define STATE_DATALEN1  5
#define STATE_DATALEN2  6
#define STATE_DATA      7
#define STATE_CHECKSUM1 8
#define STATE_CHECKSUM2 9

    switch(comms->decode_state)
    {
        case STATE_START1:
            if(byte == START_BYTE1)
            {
                comms->decode_state = STATE_START2;
                fletcher_checksum_add_byte_rx(comms, byte);
            }
            break;

        case STATE_START2:
            if(byte == START_BYTE2)
            {
                comms->decode_state = STATE_ID1;
                fletcher_checksum_add_byte_rx(comms, byte);
            }
            else
            {
                comms->decode_state = STATE_START1;
                fletcher_checksum_clear_rx(comms);
            }
            break;

        case STATE_ID1:
            comms->decode_id = byte;
            comms->decode_state = STATE_ID2;
            fletcher_checksum_add_byte_rx(comms, byte);
            break;

        case STATE_ID2:
            comms->decode_id = (comms->decode_id << 8) | byte;
            comms->decode_state = STATE_CHANNEL;
            fletcher_checksum_add_byte_rx(comms, byte);
            break;

        case STATE_CHANNEL:
            comms->decode_channel = (comms_channel_t) byte;
            if(comms->decode_channel < CHANNEL_NUM_CHANNELS)
            {
                comms->decode_state = STATE_DATALEN1;
                fletcher_checksum_add_byte_rx(comms, byte);
            }
            else
            {
                comms->decode_state = STATE_START1;
                fletcher_checksum_clear_rx(comms);
            }
            break;

        case STATE_DATALEN1:
            comms->decode_data_len = byte;
            comms->decode_state = STATE_DATALEN2;
            fletcher_checksum_add_byte_rx(comms, byte);
            break;

        case STATE_DATALEN2:
            comms->decode_data_len = (comms->decode_data_len << 8) | byte;
            if(comms->decode_data_len < comms->buf_len_rx)
            {
                if(comms->decode_data_len == 0)
                    comms->decode_state = STATE_CHECKSUM1;
                else
                {
                    comms->decode_state = STATE_DATA;
                    comms->decode_num_data_read = 0;
                }
                fletcher_checksum_add_byte_rx(comms, byte);
            }
            else
            {
                comms->decode_state = STATE_START1;
                fletcher_checksum_clear_rx(comms);
            }
            break;

        case STATE_DATA:
            comms->buf_rx[comms->decode_num_data_read++] = byte;
            fletcher_checksum_add_byte_rx(comms, byte);
            if(comms->decode_num_data_read == comms->decode_data_len)
            {
                comms->decode_state = STATE_CHECKSUM1;
            }
            break;

        case STATE_CHECKSUM1:
            comms->decode_checksum = byte;
            comms->decode_state = STATE_CHECKSUM2;
            break;

        case STATE_CHECKSUM2:
            comms->decode_checksum = fletcher_checksum_calculate(comms->decode_checksum, byte);
            uint16_t calc_checksum = fletcher_checksum_calculate_rx(comms);
            if(comms->decode_checksum == calc_checksum)
            {
                uint8_t i;
                for(i = 0; i < comms->num_subscribers[comms->decode_channel]; ++i)
                {
                    subscriber_t sub = comms->subscribers[comms->decode_channel][i]->subs;
                    void *usr = comms->subscribers[comms->decode_channel][i]->usr;
                    sub(usr, comms->decode_id, comms->decode_channel,
                        comms->buf_rx, comms->decode_data_len);
                }
                for(i = 0; i < comms->num_subscribers[CHANNEL_ALL]; ++i)
                {
                    subscriber_t sub = comms->subscribers[CHANNEL_ALL][i]->subs;
                    void *usr = comms->subscribers[CHANNEL_ALL][i]->usr;
                    sub(usr, comms->decode_id, comms->decode_channel,
                        comms->buf_rx, comms->decode_data_len);
                }
            }
            comms->decode_state = STATE_START1;
            fletcher_checksum_clear_rx(comms);
            break;
    }

#undef STATE_START1
#undef STATE_START2
#undef STATE_ID1
#undef STATE_ID2
#undef STATE_CHANNEL
#undef STATE_DATALEN1
#undef STATE_DATALEN2
#undef STATE_DATA
#undef STATE_CHECKSUM1
#undef STATE_CHECKSUM2
}

void comms_destroy(comms_t *comms)
{
    uint8_t i, j;
    for(i = 0; i < CHANNEL_NUM_CHANNELS; ++i)
    {
        for(j = 0; j < comms->num_subscribers[i]; ++j)
            free(comms->subscribers[i][j]);
        free(comms->subscribers[i]);
    }
    free(comms->publishers);
    free(comms->buf_tx);
    free(comms->buf_rx);
    free(comms);
}




static void fletcher_checksum_clear_rx(comms_t *comms)
{
    comms->checksum_rx1 = comms->checksum_rx2 = 0;
}

static void fletcher_checksum_add_byte_rx(comms_t *comms, uint8_t byte)
{
    // fletcher checksum calc
    comms->checksum_rx1 += byte;
    comms->checksum_rx2 += comms->checksum_rx1;
}

static uint16_t fletcher_checksum_calculate_rx(comms_t *comms)
{
    return fletcher_checksum_calculate(comms->checksum_rx1, comms->checksum_rx2);
}


static void fletcher_checksum_clear_tx(comms_t *comms)
{
    comms->checksum_tx1 = comms->checksum_tx2 = 0;
}

static void fletcher_checksum_add_byte_tx(comms_t *comms, uint8_t byte)
{
    // fletcher checksum calc
    comms->checksum_tx1 += byte;
    comms->checksum_tx2 += comms->checksum_tx1;
}

static uint16_t fletcher_checksum_calculate_tx(comms_t *comms)
{
    return fletcher_checksum_calculate(comms->checksum_tx1, comms->checksum_tx2);
}


static inline uint16_t fletcher_checksum_calculate(uint8_t checksum1, uint8_t checksum2)
{
    return ((uint16_t)checksum1 << 8) + (uint16_t)checksum2;
}
