/*
 * timer_capture_generate.c
 *
 *  Created on: Oct 2, 2014
 *      Author: Isaac
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"

#include "time_util.h"
#include "usb_comms.h"
#include "timer_capture_generate.h"

static uint32_t timer_bases[NUM_TIMER_BASES] = { TIMER0_BASE,  TIMER1_BASE,  TIMER2_BASE,
                                                 TIMER3_BASE,  TIMER4_BASE,  TIMER5_BASE,
                                                WTIMER0_BASE, WTIMER1_BASE, WTIMER2_BASE,
                                                WTIMER3_BASE, WTIMER4_BASE, WTIMER5_BASE};

static uint32_t timer_sys_periphs[NUM_TIMER_BASES] = { SYSCTL_PERIPH_TIMER0,  SYSCTL_PERIPH_TIMER1,
                                                       SYSCTL_PERIPH_TIMER2,  SYSCTL_PERIPH_TIMER3,
                                                       SYSCTL_PERIPH_TIMER4,  SYSCTL_PERIPH_TIMER5,
                                                      SYSCTL_PERIPH_WTIMER0, SYSCTL_PERIPH_WTIMER1,
                                                      SYSCTL_PERIPH_WTIMER2, SYSCTL_PERIPH_WTIMER3,
                                                      SYSCTL_PERIPH_WTIMER4, SYSCTL_PERIPH_WTIMER5};

static uint32_t timer_sels[NUM_TIMER_SELECTIONS] = {TIMER_A, TIMER_B};

static uint32_t gpio_ports[NUM_GPIO_PORTS] = {GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE,
                                              GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTF_BASE,
                                              GPIO_PORTG_BASE};

static uint32_t gpio_sys_periphs[NUM_GPIO_PORTS] = {SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOB,
                                                    SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD,
                                                    SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOF,
                                                    SYSCTL_PERIPH_GPIOG};

static timer_cap_gen_t default_timers[NUM_TIMERS] = {0};

static void timer_default_setup(void);

static void timer_capture_int_handler(int num);
static void timer_capture_int_handler0(void);
static void timer_capture_int_handler1(void);
static void timer_capture_int_handler2(void);
static void timer_capture_int_handler3(void);
static void timer_capture_int_handler4(void);
static void timer_capture_int_handler5(void);
static void timer_capture_int_handler6(void);
static void timer_capture_int_handler7(void);
static void timer_capture_int_handler8(void);

static void (*timer_capture_int_handlers[9])(void) = {timer_capture_int_handler0,
                                                      timer_capture_int_handler1,
                                                      timer_capture_int_handler2,
                                                      timer_capture_int_handler3,
                                                      timer_capture_int_handler4,
                                                      timer_capture_int_handler5,
                                                      timer_capture_int_handler6,
                                                      timer_capture_int_handler7,
                                                      timer_capture_int_handler8};

void timer_default_setup(void)
{
    default_timers[TIMER_INPUT_1]  = (timer_cap_gen_t){GPIO_PD3_WT3CCP1, OVERFLOW_60MS,
                                                       GPIO_PD, GPIO_PIN_3,
                                                       WTIMER3, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_2]  = (timer_cap_gen_t){GPIO_PD2_WT3CCP0, OVERFLOW_60MS,
                                                       GPIO_PD, GPIO_PIN_2,
                                                       WTIMER3, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_3]  = (timer_cap_gen_t){GPIO_PD1_WT2CCP1, OVERFLOW_60MS,
                                                       GPIO_PD, GPIO_PIN_1,
                                                       WTIMER2, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_4]  = (timer_cap_gen_t){GPIO_PB6_T0CCP0, OVERFLOW_60MS,
                                                       GPIO_PB, GPIO_PIN_6,
                                                       TIMER0, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_5]  = (timer_cap_gen_t){GPIO_PB4_T1CCP0, OVERFLOW_60MS,
                                                       GPIO_PB, GPIO_PIN_4,
                                                       TIMER1, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_6]  = (timer_cap_gen_t){GPIO_PB5_T1CCP1, OVERFLOW_60MS,
                                                       GPIO_PB, GPIO_PIN_5,
                                                       TIMER1, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_7]  = (timer_cap_gen_t){GPIO_PD6_WT5CCP0, OVERFLOW_60MS,
                                                       GPIO_PD, GPIO_PIN_6,
                                                       WTIMER5, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_8]  = (timer_cap_gen_t){GPIO_PB3_T3CCP1, OVERFLOW_60MS,
                                                       GPIO_PB, GPIO_PIN_3,
                                                       TIMER3, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_9]  = (timer_cap_gen_t){GPIO_PB2_T3CCP0, OVERFLOW_60MS,
                                                       GPIO_PB, GPIO_PIN_2,
                                                       TIMER3, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_OUTPUT_1] = (timer_cap_gen_t){GPIO_PG1_T4CCP1, OVERFLOW_20MS,
                                                       GPIO_PG, GPIO_PIN_4,
                                                       TIMER4, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_2] = (timer_cap_gen_t){GPIO_PG2_T5CCP0, OVERFLOW_20MS,
                                                       GPIO_PG, GPIO_PIN_2,
                                                       TIMER5, TIMERA, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_3] = (timer_cap_gen_t){GPIO_PG3_T5CCP1, OVERFLOW_20MS,
                                                       GPIO_PG, GPIO_PIN_3,
                                                       TIMER5, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_4] = (timer_cap_gen_t){GPIO_PC4_WT0CCP0, OVERFLOW_20MS,
                                                       GPIO_PC, GPIO_PIN_4,
                                                       WTIMER0, TIMERA, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_5] = (timer_cap_gen_t){GPIO_PC5_WT0CCP1, OVERFLOW_20MS,
                                                       GPIO_PC, GPIO_PIN_5,
                                                       WTIMER0, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_6] = (timer_cap_gen_t){GPIO_PC6_WT1CCP0, OVERFLOW_20MS,
                                                       GPIO_PC, GPIO_PIN_6,
                                                       WTIMER1, TIMERA, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_7] = (timer_cap_gen_t){GPIO_PC7_WT1CCP1, OVERFLOW_20MS,
                                                       GPIO_PC, GPIO_PIN_7,
                                                       WTIMER1, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_8] = (timer_cap_gen_t){GPIO_PB7_T0CCP1, OVERFLOW_20MS,
                                                       GPIO_PB, GPIO_PIN_7,
                                                       TIMER0, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_9] = (timer_cap_gen_t){GPIO_PG0_T4CCP0, OVERFLOW_20MS,
                                                       GPIO_PG, GPIO_PIN_0,
                                                       TIMER4, TIMERA, CAPGEN_MODE_GEN_PPM};
}

uint32_t timer_sel_to_int_cap_event(timer_cap_gen_t *timer)
{
    switch (timer->timer_sel_ind) {
        case TIMERA:
            return TIMER_CAPA_EVENT;
        case TIMERB:
            return TIMER_CAPB_EVENT;
        default:
            return 0;
    }
}

void timer_default_init(void)
{
    timer_default_setup();
    timer_init(default_timers, NUM_TIMERS);
}

void timer_init(timer_cap_gen_t timers[], uint8_t num)
{
    uint8_t i;

    // XXX can we put an ifdef DEBUG or something here?
    // Input check
    for (i=0; i<num; ++i) {
        if (timers[i].gpio_port_ind >= NUM_GPIO_PORTS) while (1);
        if (timers[i].timer_base_ind >= NUM_TIMER_BASES) while (1);
        if (timers[i].timer_sel_ind >= NUM_TIMER_SELECTIONS) while (1);
        if (    (timers[i].capgen_mode & CAPGEN_MODE_CAP_MASK)
             && (timers[i].capgen_mode & CAPGEN_MODE_GEN_MASK)) while (1);
        if (    !(timers[i].capgen_mode & CAPGEN_MODE_CAP_MASK)
             && !(timers[i].capgen_mode & CAPGEN_MODE_GEN_MASK)) while (1);
    }

    // Enable system peripherals
    for (i=0; i<num; ++i) {
        SysCtlPeripheralEnable(gpio_sys_periphs[timers[i].gpio_port_ind]);
        SysCtlPeripheralEnable(timer_sys_periphs[timers[i].timer_base_ind]);
    }

    // Configure GPIOs
    for (i=0; i<num; ++i) {
        GPIOPinConfigure(timers[i].gpio_pin_config);
        GPIOPinTypeTimer(gpio_ports[timers[i].gpio_port_ind], timers[i].gpio_pin);
    }
    for (i=0; i<num; ++i) {
        if (timers[i].capgen_mode & CAPGEN_MODE_CAP_MASK) {
            GPIOPadConfigSet(gpio_ports[timers[i].gpio_port_ind], timers[i].gpio_pin,
                             GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPD);
        }
    }

    // Configure timers
    uint32_t timer_cfg[NUM_TIMER_BASES] = {0};
    for (i=0; i<num; ++i) {
        switch (timers[i].timer_sel_ind) {
            case TIMERA:
                switch (timers[i].capgen_mode) {
                    case CAPGEN_MODE_CAP_R_EDGE:
                    case CAPGEN_MODE_CAP_F_EDGE:
                    case CAPGEN_MODE_CAP_RF_EDGE:
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME;
                        break;
                    case CAPGEN_MODE_GEN_PWM:
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM;
                        break;
                    case CAPGEN_MODE_GEN_PPM:
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT;
                        break;
                    default:
                        break;
                }
                break;
            case TIMERB:
                switch (timers[i].capgen_mode) {
                    case CAPGEN_MODE_CAP_R_EDGE:
                    case CAPGEN_MODE_CAP_F_EDGE:
                    case CAPGEN_MODE_CAP_RF_EDGE:
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_CAP_TIME;
                        break;
                    case CAPGEN_MODE_GEN_PWM:
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM;
                        break;
                    case CAPGEN_MODE_GEN_PPM:
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
    }
    for (i=0; i<NUM_TIMER_BASES; ++i) {
        if (timer_cfg[i]) TimerConfigure(timer_bases[i], timer_cfg[i]);
    }

    // Calculate load and prescale
    uint32_t sysclk_cycles_per_second = SysCtlClockGet();
    uint64_t sysclk_cycles_per_overflow;
    uint32_t prescaler;
    uint32_t load;
    for (i = 0; i < num; ++i) {
        sysclk_cycles_per_overflow = (   (uint64_t) timers[i].us_per_overflow
                                       * (uint64_t) sysclk_cycles_per_second) / 1000000;
        if (timers[i].timer_base_ind <= TIMER5) { // 16 bit timer with 8 bit prescaler
            prescaler = (sysclk_cycles_per_overflow >> 16) & 0x0000FFFF;
            if (prescaler > 0x000000FF) while (1);
            load = sysclk_cycles_per_overflow & 0x0000FFFF;
        } else { // 32 bit timer with 16 bit prescaler
            prescaler = (sysclk_cycles_per_overflow >> 32) & 0x00000000FFFFFFFF;
            if (prescaler > 0x0000FFFF) while (1);
            load = sysclk_cycles_per_overflow & 0x00000000FFFFFFFF;
        }
        TimerPrescaleSet(timer_bases[timers[i].timer_base_ind],
                         timer_sels[timers[i].timer_sel_ind], prescaler);
        TimerLoadSet(timer_bases[timers[i].timer_base_ind],
                     timer_sels[timers[i].timer_sel_ind], load);
    }

    // Configure capture events and interrupts on the inputs and match update mode on the outputs
    uint32_t mode;
    for (i=0; i<num; ++i) {
        if (timers[i].capgen_mode & CAPGEN_MODE_CAP_MASK) { // capture mode
            switch (timers[i].capgen_mode) {
                case CAPGEN_MODE_CAP_R_EDGE:
                    mode = TIMER_EVENT_POS_EDGE;
                    break;
                case CAPGEN_MODE_CAP_F_EDGE:
                    mode = TIMER_EVENT_NEG_EDGE;
                    break;
                case CAPGEN_MODE_CAP_RF_EDGE:
                    mode = TIMER_EVENT_BOTH_EDGES;
                    break;
                default:
                    while (1);
            }
            TimerControlEvent(timer_bases[timers[i].timer_base_ind],
                              timer_sels[timers[i].timer_sel_ind],
                              mode);

            mode = timer_sel_to_int_cap_event(&timers[i]);
            TimerIntEnable(timer_bases[timers[i].timer_base_ind], mode);
            IntPrioritySet(_TimerIntNumberGet(timer_bases[timers[i].timer_base_ind],
                                              timer_sels[timers[i].timer_sel_ind]), 0x20);
            // XXX: make a good interrupt handler
            TimerIntRegister(timer_bases[timers[i].timer_base_ind],
                             timer_bases[timers[i].timer_sel_ind],
                             timer_capture_int_handlers[i]);
        } else { // generator mode
            switch (timers[i].capgen_mode) {
                case CAPGEN_MODE_GEN_PWM:
                    mode = TIMER_UP_LOAD_TIMEOUT | TIMER_UP_MATCH_TIMEOUT;
                    break;
                case CAPGEN_MODE_GEN_PPM:
                    mode = TIMER_UP_LOAD_IMMEDIATE | TIMER_UP_MATCH_IMMEDIATE;
                    break;
                default:
                    while (1);
            }
            TimerUpdateMode(timer_bases[timers[i].timer_base_ind],
                            timer_sels[timers[i].timer_sel_ind],
                            mode);
        }
    }

    // Enable timers
    for (i=0; i<num; ++i) {
        TimerEnable(timer_bases[timers[i].timer_base_ind], timer_sels[timers[i].timer_sel_ind]);
    }
}

void timer_generate_pulse_percent(float percent)
{
    // XXX fill out
}

void timer_generate_pulse(uint32_t pulse_width)
{
    uint32_t prescaler;
    uint32_t load;
    uint64_t total_load;
    uint64_t total_match;
    uint32_t prescaler_match;
    uint32_t match;

    uint8_t i;
    for (i=TIMER_OUTPUT_1; i<=TIMER_OUTPUT_8; ++i) {
        prescaler = TimerPrescaleGet(timer_bases[default_timers[i].timer_base_ind],
                                     timer_sels[default_timers[i].timer_sel_ind]);
        load = TimerLoadGet(timer_bases[default_timers[i].timer_base_ind],
                            timer_sels[default_timers[i].timer_sel_ind]);

        if (default_timers[i].timer_base_ind <= TIMER5) { // 16 bit timer with 8 bit prescaler
            total_load = ((prescaler << 16) & 0x00FF0000) + (load & 0x0000FFFF);
        } else { // 32 bit timer with 16 bit prescaler
            total_load =   ((((uint64_t)prescaler) << 32) & 0x0000FFFF00000000)
                         + (load & 0x00000000FFFFFFFF);
        }

        if (pulse_width > total_load) total_match = 0;
        else if (pulse_width == 0) total_match = total_load - 1;
        else total_match = total_load - pulse_width;

        if (default_timers[i].timer_base_ind <= TIMER5) { // 16 bit timer with 8 bit prescaler
            prescaler_match = (total_match >> 16) & 0x000000FF;
            match = total_match & 0x0000FFFF;
        } else { // 32 bit timer with 16 bit prescaler
            prescaler_match = (total_match >> 32) & 0x000000000000FFFF;
            match = total_match & 0x00000000FFFFFFFF;
        }

        TimerPrescaleMatchSet(timer_bases[default_timers[i].timer_base_ind],
                              timer_sels[default_timers[i].timer_sel_ind],
                              prescaler_match);
        TimerMatchSet(timer_bases[default_timers[i].timer_base_ind],
                      timer_sels[default_timers[i].timer_sel_ind],
                      match);
    }
}

static void timer_capture_int_handler0(void)
{
    timer_capture_int_handler(0);
}

static void timer_capture_int_handler1(void)
{
    timer_capture_int_handler(1);
}

static void timer_capture_int_handler2(void)
{
    timer_capture_int_handler(2);
}

static void timer_capture_int_handler3(void)
{
    timer_capture_int_handler(3);
}

static void timer_capture_int_handler4(void)
{
    timer_capture_int_handler(4);
}

static void timer_capture_int_handler5(void)
{
    timer_capture_int_handler(5);
}

static void timer_capture_int_handler6(void)
{
    timer_capture_int_handler(6);
}

static void timer_capture_int_handler7(void)
{
    timer_capture_int_handler(7);
}

static void timer_capture_int_handler8(void)
{
    timer_capture_int_handler(8);
}

static void timer_capture_int_handler(int num)
{
    static uint8_t msg[30];
    static uint64_t lastTime = UINT64_MAX;
    uint32_t timer_base = timer_bases[default_timers[num].timer_base_ind];
    uint32_t timer_sel = timer_sels[default_timers[num].timer_sel_ind];
    uint32_t timer_int = timer_sel_to_int_cap_event(&default_timers[num]);
    uint64_t time;
    uint32_t prescaler;
    uint32_t load;
    uint64_t total_load;

    TimerIntClear(timer_base, timer_int);
    time = TimerValueGet(timer_base, timer_sel);
    prescaler = TimerPrescaleGet(timer_base, timer_sel);
    load = TimerLoadGet(timer_base, timer_sel);
    if (default_timers[num].timer_base_ind <= TIMER5) { // 16 bit timer with 8 bit prescaler
        total_load = ((prescaler << 16) & 0x00FF0000) + (load & 0x0000FFFF);
    } else {  // 32 bit timer with 16 bit prescaler
        total_load =   ((((uint64_t)prescaler) << 32) & 0x0000FFFF00000000)
                     + (load & 0x00000000FFFFFFFF);
    }

    if (lastTime < UINT64_MAX) {
        uint32_t delta;
        if(lastTime < time) delta = lastTime + total_load - time;
        else delta = lastTime - time;
        usb_write(msg, snprintf((char*)msg, 30, "%lu\r\n", delta/80));
    }
    lastTime = time;
}
