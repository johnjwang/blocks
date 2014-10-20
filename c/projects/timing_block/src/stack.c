/*
 * stack.c
 *
 *  Created on: Oct 10, 2014
 *      Author: Jonathan
 */
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"

#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

#include "stack.h"
#include "timer_capture_generate.h"

static void receive_pulse(void);
static void timeout_start_enumeration(void);
static void enumerate_above(void);
static void still_enumertaing(void);
static void finish_enumeration(void);

#define STARTED_ENUMERATION 1
#define FINISHED_ENUMERATION 2

stack_t stack;

void __stack_init(void)
{
    SysCtlPeripheralEnable(stack.sysctl_gpio_up_port_base);
    SysCtlPeripheralEnable(stack.sysctl_gpio_dn_port_base);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);
    SysCtlDelay(1);

    stack.enumeration_state = 0;
    stack.address = 0;
    stack.num_toggles_sent = 0;

    // First weakly pull down your bottom gpio on the stacks and set up their interrupts for pulse capturing
    GPIOPinTypeGPIOInput(stack.gpio_dn_port_base, stack.gpio_dn_pin);
    GPIOPadConfigSet(stack.gpio_dn_port_base, stack.gpio_dn_pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
    // Configure pins to capture falling edge
    GPIOIntTypeSet(stack.gpio_dn_port_base, stack.gpio_dn_pin, GPIO_FALLING_EDGE);
    GPIOIntEnable(stack.gpio_dn_port_base, stack.gpio_dn_pin_int);
    IntPrioritySet(stack.gpio_dn_port_int, 0x10);
    // XXX Need to write a gpio controller that handles callbacks of different gpio interrupts
    GPIOIntRegister(stack.gpio_dn_port_base, receive_pulse);

    // Drive your top gpio on the stacks high
    GPIOPinTypeGPIOOutput(stack.gpio_up_port_base, stack.gpio_up_pin);
    GPIOPadConfigSet(stack.gpio_up_port_base, stack.gpio_up_pin, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
    GPIOPinWrite(stack.gpio_up_port_base, stack.gpio_up_pin, stack.gpio_up_pin);

    // Initialize the i2c bus

    // After 100ms,
    TimerConfigure(TIMER2_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC_UP | TIMER_CFG_B_ONE_SHOT_UP);
    timer_set(TIMER2_BASE, TIMER_B, timer_us_to_tics(100000), 16, 8);
    TimerIntEnable(TIMER2_BASE, TIMER_TIMB_TIMEOUT);
    IntPrioritySet(_TimerIntNumberGet(TIMER2_BASE, TIMER_B), 0x20);
    TimerIntRegister(TIMER2_BASE, TIMER_B, timeout_start_enumeration);
    TimerEnable(TIMER2_BASE, TIMER_B);
}


static void timeout_start_enumeration(void)
{
    TimerDisable(TIMER2_BASE, TIMER_B);
    TimerIntUnregister(TIMER2_BASE, TIMER_B);
    TimerIntClear(TIMER2_BASE, TIMER_TIMB_TIMEOUT);

    // check your bottom gpio.
    // If it is still low, you are at the bottom of the stack.
    // You begin the enumeration.
    bool bottomOfStack = !GPIOPinRead(stack.gpio_dn_port_base, stack.gpio_dn_pin);
    if(bottomOfStack)
    {
        // If we've already started/done the enumeration process by this point, continue
        if(stack.address != 0) return;

        GPIOIntUnregister(stack.gpio_dn_port_base);
        GPIOIntClear(stack.gpio_dn_port_base, stack.gpio_dn_pin_int);

        stack.address = 0x01;
        enumerate_above();
    }
    // else
    //      enumeration has not gotten to us yet but is in progress
}


static void receive_pulse(void)
{
    GPIOIntClear(stack.gpio_dn_port_base, stack.gpio_dn_pin_int);

    TimerDisable(TIMER2_BASE, TIMER_B);
    TimerIntClear(TIMER2_BASE, TIMER_TIMB_TIMEOUT);

    stack.address++;

    // Set up to wait for 300us before timing out the incoming pulses
    //XXX This should be moved somewhere persistent.
    //    This actually sets the value in the timer to 0.
    TimerValueSet(TIMER2_BASE, TIMER_B, 0);
    timer_set(TIMER2_BASE, TIMER_B, timer_us_to_tics(500), 16, 8);
    TimerIntRegister(TIMER2_BASE, TIMER_B, enumerate_above);
    TimerEnable(TIMER2_BASE, TIMER_B);
}


static void enumerate_above(void)
{
    TimerDisable(TIMER2_BASE, TIMER_B);
    TimerIntUnregister(TIMER2_BASE, TIMER_B);
    TimerIntClear(TIMER2_BASE, TIMER_TIMB_TIMEOUT);

    switch(stack.enumeration_state)
    {
        case 0:
            //Place yourself on the i2c bus with your new address and write your address to the bus here

            stack.enumeration_state = STARTED_ENUMERATION;
            GPIOPinTypeGPIOOutput(stack.gpio_dn_port_base, stack.gpio_dn_pin);
            GPIOPadConfigSet(stack.gpio_dn_port_base, stack.gpio_dn_pin, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
            GPIOPinWrite(stack.gpio_dn_port_base, stack.gpio_dn_pin, stack.gpio_dn_pin);
            GPIOPinWrite(stack.gpio_up_port_base, stack.gpio_up_pin, 0);
            timer_set(TIMER2_BASE, TIMER_B, timer_us_to_tics(100), 16, 8);
            TimerIntRegister(TIMER2_BASE, TIMER_B, enumerate_above);
            TimerEnable(TIMER2_BASE, TIMER_B);
            stack.num_toggles_sent++;
            break;

        case STARTED_ENUMERATION:
            GPIOPinWrite(stack.gpio_up_port_base, stack.gpio_up_pin, ~GPIOPinRead(stack.gpio_up_port_base, stack.gpio_up_pin));
            stack.num_toggles_sent++;

            if(stack.num_toggles_sent == (stack.address * 2) + 1)
            {
                GPIOPinTypeGPIOInput(stack.gpio_up_port_base, stack.gpio_up_pin);
                GPIOPadConfigSet(stack.gpio_up_port_base, stack.gpio_up_pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
                // Configure pins to capture rising edge
                //XXX Why do we need to clear the interrupt here? If we don't do this,
                //    we automatically fall into the rising edge interrupt.
                GPIOIntClear(stack.gpio_up_port_base, stack.gpio_up_pin_int);
                GPIOIntTypeSet(stack.gpio_up_port_base, stack.gpio_up_pin, GPIO_RISING_EDGE);
                GPIOIntEnable(stack.gpio_up_port_base, stack.gpio_up_pin_int);
                IntPrioritySet(stack.gpio_up_port_int, 0x10);
                GPIOIntRegister(stack.gpio_up_port_base, still_enumertaing);

                timer_set(TIMER2_BASE, TIMER_B, timer_us_to_tics(500), 16, 8);
                TimerIntRegister(TIMER2_BASE, TIMER_B, finish_enumeration);
                TimerEnable(TIMER2_BASE, TIMER_B);

                stack.enumeration_state = FINISHED_ENUMERATION;
            }
            else
            {
                timer_set(TIMER2_BASE, TIMER_B, timer_us_to_tics(100), 16, 8);
                TimerIntRegister(TIMER2_BASE, TIMER_B, enumerate_above);
                TimerEnable(TIMER2_BASE, TIMER_B);
            }
            break;

        default:
            while(1);
    }
}

static void still_enumertaing(void)
{
    GPIOIntUnregister(stack.gpio_up_port_base);
    GPIOIntClear(stack.gpio_up_port_base, stack.gpio_up_pin_int);

    TimerDisable(TIMER2_BASE, TIMER_B);
    TimerIntUnregister(TIMER2_BASE, TIMER_B);
    TimerIntClear(TIMER2_BASE, TIMER_TIMB_TIMEOUT);

    GPIOPadConfigSet(stack.gpio_up_port_base, stack.gpio_up_pin, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Configure pins to capture falling edge
    GPIOIntTypeSet(stack.gpio_up_port_base, stack.gpio_up_pin, GPIO_FALLING_EDGE);
    GPIOIntEnable(stack.gpio_up_port_base, stack.gpio_up_pin_int);
    IntPrioritySet(stack.gpio_up_port_int, 0x10);
    GPIOIntRegister(stack.gpio_up_port_base, finish_enumeration);
    if(GPIOPinRead(stack.gpio_up_port_base, stack.gpio_up_pin) == 0)
    {
        finish_enumeration();
    }
}

static void finish_enumeration(void)
{
    GPIOIntUnregister(stack.gpio_up_port_base);
    GPIOIntClear(stack.gpio_up_port_base, stack.gpio_up_pin_int);

    TimerDisable(TIMER2_BASE, TIMER_B);
    TimerIntUnregister(TIMER2_BASE, TIMER_B);
    TimerIntClear(TIMER2_BASE, TIMER_TIMB_TIMEOUT);

    GPIOPinWrite(stack.gpio_dn_port_base, stack.gpio_dn_pin, 0);

    // Do something
}
