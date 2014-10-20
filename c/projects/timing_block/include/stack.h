/*
 * stack.h
 *
 *  Created on: Oct 10, 2014
 *      Author: jonathan
 */

/* Stack Enumeration Procedure
 *      First weakly pull down your bottom gpio on the stacks
 *      Drive your top gpio on the stacks high
 *      Initialize the i2c bus
 *      After 100ms check your bottom gpio. If it is still low, you are at the bottom of the stack. You begin the enumeration.
 *      Assign yourself an address. And pulse your address + 1 to the chip above you. If you are the bottom of the stack, your address is 1 and you pulse a 2 upwards.
 *          The pulsing scheme is as follows:
 *              Send the number of pulses equal to the next available stack address. Ie, if your address is 1, send 2 pulses to the chip above you.
 *              The pulses should be 100us in length and should be spaced apart by 100us.
 *              After the last pulse you send, reconfigure the top gpio pin as an input with a weak pull down.
 *      On a rising edge of the pin, you know that there is another device on the stack above you that is continuing the enumeration process.
 *      Place yourself on the i2c bus with your newly assigned address.
 *      On the falling edge of the same pin, you know that enumeration is complete. Drive your bottom gpio pin low.
 */

#ifndef STACK_H_
#define STACK_H_

#include <stdint.h>
#include <stdbool.h>

#include "driverlib/gpio.h"

typedef struct stack_t
{
    uint32_t sysctl_gpio_up_port_base;
    uint32_t gpio_up_port_base;
    uint32_t gpio_up_pin;
    uint32_t gpio_up_port_int;
    uint32_t gpio_up_pin_int;

    uint32_t sysctl_gpio_dn_port_base;
    uint32_t gpio_dn_port_base;
    uint32_t gpio_dn_pin;
    uint32_t gpio_dn_port_int;
    uint32_t gpio_dn_pin_int;

    volatile uint16_t address;
    uint16_t num_toggles_sent;
    uint8_t  enumeration_state;

} stack_t;

extern stack_t stack;

#define stack_enumerate(GPIO_UP_PORT, GPIO_UP_PIN, GPIO_DN_PORT, GPIO_DN_PIN)\
    do {\
    	stack.sysctl_gpio_up_port_base = SYSCTL_PERIPH_GPIO ## GPIO_UP_PORT; \
        stack.gpio_up_port_base        = GPIO_PORT ## GPIO_UP_PORT ## _BASE; \
    	stack.gpio_up_pin              = GPIO_PIN_ ## GPIO_UP_PIN; \
        stack.gpio_up_port_int         = INT_GPIO ## GPIO_UP_PORT; \
        stack.gpio_up_pin_int          = GPIO_INT_PIN_ ## GPIO_UP_PIN; \
        \
        stack.sysctl_gpio_dn_port_base = SYSCTL_PERIPH_GPIO ## GPIO_DN_PORT; \
        stack.gpio_dn_port_base        = GPIO_PORT ## GPIO_DN_PORT ## _BASE; \
        stack.gpio_dn_pin              = GPIO_PIN_ ## GPIO_DN_PIN; \
        stack.gpio_dn_port_int         = INT_GPIO ## GPIO_DN_PORT; \
        stack.gpio_dn_pin_int          = GPIO_INT_PIN_ ## GPIO_DN_PIN; \
        \
        __stack_init(); \
    } while(0)

void __stack_init(void);

#endif /* STACK_H_ */
