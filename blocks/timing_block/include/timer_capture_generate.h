/*
 * timer_capture_generate.h
 *
 *  Created on: Oct 2, 2014
 *      Author: Isaac
 */

#ifndef TIMER_CAPTURE_GENERATE_H_
#define TIMER_CAPTURE_GENERATE_H_

typedef struct _timer_cap_gen_t
{

} timer_cap_gen_t;

void timer_capture_generate_init(void);
void timer_generate_pulse(uint32_t pulse_width);
void timer_generate_pulse_percent(float percent);
void timer_capture_generate_start(void);

#endif /* TIMER_CAPTURE_GENERATE_H_ */
