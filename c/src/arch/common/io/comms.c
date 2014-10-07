/*
 * comms.c
 *
 *  Created on: Oct 5, 2014
 *      Author: Jonathan
 */
#include <stdlib.h>
#include <stdint.h>

#include "io/comms.h"

#define START_BYTE1 0xB1
#define START_BYTE2 0x75

static void fletcher_checksum_clear_rx(comms_t *comms);
static void fletcher_checksum_add_byte_rx(comms_t *comms, uint8_t byte);
static uint16_t fletcher_checksum_calculate_rx(comms_t *comms);
static void fletcher_checksum_clear_tx(comms_t *comms);
static void fletcher_checksum_add_byte_tx(comms_t *comms, uint8_t byte);
static uint16_t fletcher_checksum_calculate_tx(comms_t *comms);
static inline uint16_t fletcher_checksum_calculate(uint8_t checksum1, uint8_t checksum2);

comms_t* comms_create(publisher_t publisher, int32_t decode_buf_len)
{
    comms_t *ret = (comms_t*) malloc(sizeof(comms_t));
    if(ret == NULL) return NULL;

    ret->publisher = publisher;

    ret->decode_buf_len = decode_buf_len;
    ret->decode_buf = (uint8_t*) malloc(sizeof(uint8_t) * decode_buf_len);
    ret->decode_state = 0;
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

void comms_subscribe(comms_t *comms, comms_channel_t channel, subscriber_t subscriber)
{
    if(channel >= CHANNEL_NUM_CHANNELS) while(1);
    comms->num_subscribers[channel]++;
    comms->subscribers[channel] = realloc(comms->subscribers[channel],
                                          comms->num_subscribers[channel] *
                                          sizeof(subscriber_t));
    comms->subscribers[channel][comms->num_subscribers[channel] - 1] = subscriber;
}

void comms_publish_blocking(comms_t *comms, comms_channel_t channel, uint8_t *msg, uint16_t msg_len)
{
    fletcher_checksum_clear_tx(comms);

    fletcher_checksum_add_byte_tx(comms, START_BYTE1);
    comms->publisher(START_BYTE1);

    fletcher_checksum_add_byte_tx(comms, START_BYTE2);
    comms->publisher(START_BYTE2);

    fletcher_checksum_add_byte_tx(comms, channel);
    comms->publisher(channel);

    uint8_t len1 = (msg_len & 0xff00) >> 8;
    fletcher_checksum_add_byte_tx(comms, len1);
    comms->publisher(len1);

    uint8_t len2 = msg_len & 0x00ff;
    fletcher_checksum_add_byte_tx(comms, len2);
    comms->publisher(len2);

    uint8_t i;
    for(i = 0; i< msg_len; ++i)
    {
        fletcher_checksum_add_byte_tx(comms, msg[i]);
        comms->publisher(msg[i]);
    }

    fletcher_checksum_calculate_tx(comms);

    comms->publisher(comms->checksum_tx1);
    comms->publisher(comms->checksum_tx2);
}

void comms_handle(comms_t *comms, uint8_t byte)
{
#define STATE_START1    0
#define STATE_START2    1
#define STATE_CHANNEL   2
#define STATE_DATALEN1  3
#define STATE_DATALEN2  4
#define STATE_DATA      5
#define STATE_CHECKSUM1 6
#define STATE_CHECKSUM2 7

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
                comms->decode_state = STATE_CHANNEL;
                fletcher_checksum_add_byte_rx(comms, byte);
            }
            else
            {
                comms->decode_state = STATE_START1;
                fletcher_checksum_clear_rx(comms);
            }
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
            if(comms->decode_data_len < comms->decode_buf_len)
            {
                comms->decode_state = STATE_DATA;
                comms->decode_num_data_read = 0;
                fletcher_checksum_add_byte_rx(comms, byte);
            }
            else
            {
                comms->decode_state = STATE_START1;
                fletcher_checksum_clear_rx(comms);
            }
            break;

        case STATE_DATA:
            comms->decode_buf[comms->decode_num_data_read++] = byte;
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
            if(comms->decode_checksum == fletcher_checksum_calculate_rx(comms))
            {
                uint8_t i;
                for(i = 0; i < comms->num_subscribers[comms->decode_channel]; ++i)
                {
                    subscriber_t sub = comms->subscribers[comms->decode_channel][i];
                    sub(comms->decode_buf, comms->decode_data_len);
                }
            }
            comms->decode_state = STATE_START1;
            fletcher_checksum_clear_rx(comms);
            break;
    }

#undef STATE_START1
#undef STATE_START2
#undef STATE_CHANNEL
#undef STATE_DATALEN1
#undef STATE_DATALEN2
#undef STATE_DATA
#undef STATE_CHECKSUM1
#undef STATE_CHECKSUM2
}

void comms_destroy(comms_t *comms)
{
    uint8_t i;
    for(i = 0; i < CHANNEL_NUM_CHANNELS; ++i)
    {
        free(comms->subscribers[i]);
    }
    free(comms->decode_buf);
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
