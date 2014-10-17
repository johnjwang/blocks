/*
 * comms.c
 *
 *  Created on: Oct 5, 2014
 *      Author: Jonathan
 */
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "io/comms.h"
#include "datastruct/circular.h"

typedef struct subscriber_container_t
{
    subscriber_t subs;
    void *usr;

} subscriber_container_t;


struct comms_t
{
    uint8_t *buf_rx;
    uint16_t buf_rx_len;

    container_t *buf_tx;

    publisher_t publisher;

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

container_funcs_t *comms_cfuncs;

comms_t* comms_create(uint32_t buf_rx_len, uint32_t buf_tx_len,
                      publisher_t publisher)
{
    comms_t *ret = (comms_t*) malloc(sizeof(comms_t));
    if(ret == NULL) return NULL;
    memset(ret, 0.0, sizeof(comms_t));

    circular_funcs_init();
    comms_cfuncs = &circular_funcs;

    ret->buf_rx_len = buf_rx_len;
    ret->buf_rx = (uint8_t*) calloc(ret->buf_rx_len, sizeof(uint8_t));
    if(ret->buf_rx == NULL)
    {
        comms_destroy(ret);
        return NULL;
    }
    // This makes our lives easier when handling
    // the buffering of messages to be published
    if(buf_tx_len == 0) buf_tx_len = 1;
    ret->buf_tx = comms_cfuncs->create(buf_tx_len, sizeof(uint8_t));
    if(ret->buf_tx == NULL)
    {
        comms_destroy(ret);
        return NULL;
    }

    ret->publisher = publisher;

    ret->decode_state = 0;
    ret->decode_id = 0;
    ret->decode_channel = (comms_channel_t) 0;
    ret->decode_data_len = 0;
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

static comms_status_t publish_flush(comms_t *comms)
{
    if(!comms_cfuncs->is_empty(comms->buf_tx))
        comms->publisher(comms->buf_tx);

    if(comms_cfuncs->is_empty(comms->buf_tx))
        return COMMS_STATUS_DONE;
    else
        return COMMS_STATUS_IN_PROGRESS;

}

static comms_status_t publish(comms_t *comms, uint8_t data)
{
    if(!comms_cfuncs->push_back(comms->buf_tx, &data))
        return COMMS_STATUS_BUFFER_FULL;

    if(comms_cfuncs->is_full(comms->buf_tx))
        return publish_flush(comms);
}

inline comms_status_t comms_publish(comms_t *comms,
                                   comms_channel_t channel,
                                   uint8_t *msg,
                                   uint16_t msg_len)
{
    return comms_publish_id(comms, 0, channel, msg, msg_len);
}

comms_status_t comms_publish_id(comms_t *comms,
                               uint16_t id,
                               comms_channel_t channel,
                               uint8_t *msg,
                               uint16_t msg_len)
{
#define NUM_METADATA 1+1+2+1+2+2

    fletcher_checksum_clear_tx(comms);

    fletcher_checksum_add_byte_tx(comms, START_BYTE1);
    if(publish(comms, START_BYTE1) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    fletcher_checksum_add_byte_tx(comms, START_BYTE2);
    if(publish(comms, START_BYTE2) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    uint8_t id1 = (id & 0xff00) >> 8;
    fletcher_checksum_add_byte_tx(comms, id1);
    if(publish(comms, id1) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    uint8_t id2 = id & 0x00ff;
    fletcher_checksum_add_byte_tx(comms, id2);
    if(publish(comms, id2) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    fletcher_checksum_add_byte_tx(comms, channel);
    if(publish(comms, channel) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    uint8_t len1 = (msg_len & 0xff00) >> 8;
    fletcher_checksum_add_byte_tx(comms, len1);
    if(publish(comms, len1) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    uint8_t len2 = msg_len & 0x00ff;
    fletcher_checksum_add_byte_tx(comms, len2);
    if(publish(comms, len2) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    uint8_t i;
    for(i = 0; i< msg_len; ++i)
    {
        fletcher_checksum_add_byte_tx(comms, msg[i]);
        if(publish(comms, msg[i]) == COMMS_STATUS_BUFFER_FULL)
            return COMMS_STATUS_BUFFER_FULL;
    }

    fletcher_checksum_calculate_tx(comms);

    if(publish(comms, comms->checksum_tx1) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;
    if(publish(comms, comms->checksum_tx2) == COMMS_STATUS_BUFFER_FULL)
        return COMMS_STATUS_BUFFER_FULL;

    return publish_flush(comms);

#undef NUM_METADATA
}

inline comms_status_t comms_transmit(comms_t *comms)
{
    return publish_flush(comms);
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

try_again:

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
                goto try_again;
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
                goto try_again;
            }
            break;

        case STATE_DATALEN1:
            comms->decode_data_len = byte;
            comms->decode_state = STATE_DATALEN2;
            fletcher_checksum_add_byte_rx(comms, byte);
            break;

        case STATE_DATALEN2:
            comms->decode_data_len = (comms->decode_data_len << 8) | byte;
            if(comms->decode_data_len < comms->buf_rx_len)
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
                goto try_again;
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
            comms->decode_state = STATE_START1;
            fletcher_checksum_clear_rx(comms);
            if(comms->decode_checksum == calc_checksum)
            {
                uint8_t i;
                for(i = 0; i < comms->num_subscribers[comms->decode_channel]; ++i)
                {
                    subscriber_t sub = comms->subscribers[comms->decode_channel][i]->subs;
                    void *usr = comms->subscribers[comms->decode_channel][i]->usr;
                    sub(usr, comms->decode_id, comms->decode_channel, comms->buf_rx, comms->decode_data_len);
                }
                for(i = 0; i < comms->num_subscribers[CHANNEL_ALL]; ++i)
                {
                    subscriber_t sub = comms->subscribers[CHANNEL_ALL][i]->subs;
                    void *usr = comms->subscribers[CHANNEL_ALL][i]->usr;
                    sub(usr, comms->decode_id, comms->decode_channel, comms->buf_rx, comms->decode_data_len);
                }
            }
            else
                goto try_again;
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
        if(comms->subscribers[i] != NULL)
            free(comms->subscribers[i]);
    }

    if(comms->buf_tx)
        comms_cfuncs->destroy(comms->buf_tx);
    if(comms->buf_rx)
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
