/*
 * timing_block_util.h
 *
 *  Created on: Sep 29, 2014
 *      Author: Jonathan
 */

#ifndef TIMING_BLOCK_UTIL_H_
#define TIMING_BLOCK_UTIL_H_

#include <stdint.h>

void leds_init();
bool is_blinking(uint8_t led_num);
bool blink_led(uint8_t led_num, uint8_t num_blinks);
bool turn_on_led(uint8_t led_num);
bool turn_off_led(uint8_t led_num);
bool toggle_led(uint8_t led_num);

#endif /* TIMING_BLOCK_UTIL_H_ */
