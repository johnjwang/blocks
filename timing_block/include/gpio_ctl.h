/*
 * gpio_ctl.h
 *
 *  Created on: Oct 2, 2014
 *      Author: Isaac Olson
 */

#ifndef GPIO_CTL_H_
#define GPIO_CTL_H_

#define NUM_GPIO 18

#define GPIO_INPUT_1 0   /*D3*/
#define GPIO_INPUT_2 1   /*D2*/
#define GPIO_INPUT_3 2   /*D1*/
#define GPIO_INPUT_4 3   /*B6*/
#define GPIO_INPUT_5 4   /*B4*/
#define GPIO_INPUT_6 5   /*B5*/
#define GPIO_INPUT_7 6   /*D6*/
#define GPIO_INPUT_8 7   /*B3*/
#define GPIO_INPUT_9 8   /*B2*/

#define GPIO_OUTPUT_1 9  /*G1*/
#define GPIO_OUTPUT_2 10 /*G2*/
#define GPIO_OUTPUT_3 11 /*G3*/
#define GPIO_OUTPUT_4 12 /*C4*/
#define GPIO_OUTPUT_5 13 /*C5*/
#define GPIO_OUTPUT_6 14 /*C6*/
#define GPIO_OUTPUT_7 15 /*C7*/
#define GPIO_OUTPUT_8 16 /*B7*/
#define GPIO_OUTPUT_9 17 /*G0*/

void gpio_ctl_init();

int8_t gpio_ctl_read(uint32_t gpio_num); // gpio_num = GPIO_???PUT_n
void gpio_ctl_write(uint32_t gpio_num, uint8_t value); // gpio_num = GPIO_???PUT_n, value = 1,0
void gpio_ctl_values_snprintf(uint8_t *buf, uint32_t len);

#endif /* GPIO_CTL_H_ */
