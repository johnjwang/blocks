/*
 * comms.c
 *
 *  Created on: Oct 5, 2014
 *      Author: Jonathan
 */
#include <stdlib.h>
#include <stdint.h>

#include "comms.h"

#define START_BYTE1 0xB1
#define START_BYTE2 0x75

static void fletcher_checksum_clear(comms_t *comms);
static void fletcher_checksum_add_byte(comms_t *comms, uint8_t byte);
static uint16_t fletcher_checksum_calculate(comms_t *comms);


comms_t* comms_create(publisher_t publisher, int32_t buf_len)
{
    comms_t *ret = (comms_t*) malloc(sizeof(comms_t));

    ret->buf_len = buf_len;
    ret->buf = (uint8_t*) malloc(sizeof(uint8_t) * buf_len);

    ret->publisher = publisher;

    return ret;
}

void comms_publish_blocking(comms_t *comms, uint8_t channel, uint8_t *msg, uint16_t msg_len)
{
    fletcher_checksum_clear(comms);

    fletcher_checksum_add_byte(comms, START_BYTE1);
    comms->publisher(START_BYTE1);

    fletcher_checksum_add_byte(comms, START_BYTE2);
    comms->publisher(START_BYTE2);

    fletcher_checksum_add_byte(comms, channel);
    comms->publisher(channel);

    uint8_t len1 = (msg_len & 0xff00) >> 8;
    fletcher_checksum_add_byte(comms, len1);
    comms->publisher(len1);

    uint8_t len2 = msg_len & 0x00ff;
    fletcher_checksum_add_byte(comms, len2);
    comms->publisher(len2);

    uint8_t i = 0;
    while(i < msg_len)
    {
        fletcher_checksum_add_byte(comms, msg[i]);
        comms->publisher(msg[i]);
    }

    fletcher_checksum_calculate(comms);

    comms->publisher(comms->checksum1);
    comms->publisher(comms->checksum2);
}

static void fletcher_checksum_clear(comms_t *comms)
{
    comms->checksum1 = comms->checksum2 = 0;
}

static void fletcher_checksum_add_byte(comms_t *comms, uint8_t byte)
{
    // fletcher checksum calc
    comms->checksum1 += byte;
    comms->checksum2 += comms->checksum1;
}

static uint16_t fletcher_checksum_calculate(comms_t *comms)
{
    return ((uint16_t)comms->checksum1 << 8) + (uint16_t)comms->checksum2;
}
