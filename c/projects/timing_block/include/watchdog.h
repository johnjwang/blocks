/*
 * watchdog.h
 *
 *  Created on: Oct 10, 2014
 *      Author: Isaac Olson
 */

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

void watchdog_init(uint32_t tics_timeout);
void watchdog_feed(void);

#endif /* WATCHDOG_H_ */
