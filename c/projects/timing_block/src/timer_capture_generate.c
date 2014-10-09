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

#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"

#include "time_util.h"
#include "usb_comms.h"
#include "timer_capture_generate.h"

static const uint32_t timer_bases[NUM_TIMER_BASES] = { TIMER0_BASE,  TIMER1_BASE,  TIMER2_BASE,
                                                       TIMER3_BASE,  TIMER4_BASE,  TIMER5_BASE,
                                                      WTIMER0_BASE, WTIMER1_BASE, WTIMER2_BASE,
                                                      WTIMER3_BASE, WTIMER4_BASE, WTIMER5_BASE};

static const uint32_t timer_sys_periphs[NUM_TIMER_BASES] = { SYSCTL_PERIPH_TIMER0,  SYSCTL_PERIPH_TIMER1,
                                                             SYSCTL_PERIPH_TIMER2,  SYSCTL_PERIPH_TIMER3,
                                                             SYSCTL_PERIPH_TIMER4,  SYSCTL_PERIPH_TIMER5,
                                                            SYSCTL_PERIPH_WTIMER0, SYSCTL_PERIPH_WTIMER1,
                                                            SYSCTL_PERIPH_WTIMER2, SYSCTL_PERIPH_WTIMER3,
                                                            SYSCTL_PERIPH_WTIMER4, SYSCTL_PERIPH_WTIMER5};

static const uint32_t timer_sels[NUM_TIMER_SELECTIONS] = {TIMER_A, TIMER_B};

static const uint32_t gpio_ports[NUM_GPIO_PORTS] = {GPIO_PORTA_BASE, GPIO_PORTB_BASE, GPIO_PORTC_BASE,
                                                    GPIO_PORTD_BASE, GPIO_PORTE_BASE, GPIO_PORTF_BASE,
                                                    GPIO_PORTG_BASE};

static const uint32_t gpio_sys_periphs[NUM_GPIO_PORTS] = {SYSCTL_PERIPH_GPIOA, SYSCTL_PERIPH_GPIOB,
                                                          SYSCTL_PERIPH_GPIOC, SYSCTL_PERIPH_GPIOD,
                                                          SYSCTL_PERIPH_GPIOE, SYSCTL_PERIPH_GPIOF,
                                                          SYSCTL_PERIPH_GPIOG};

static timer_cap_gen_t default_timers[NUM_TIMERS] = {0};

static void timer_default_setup(void);

static void timer_capture_int_handler(int num);
static void timer_capture_int_handler1(void);
static void timer_capture_int_handler2(void);
static void timer_capture_int_handler3(void);
static void timer_capture_int_handler4(void);
static void timer_capture_int_handler5(void);
static void timer_capture_int_handler6(void);
static void timer_capture_int_handler7(void);
static void timer_capture_int_handler8(void);
static void (*timer_capture_int_handlers[8])(void) = {timer_capture_int_handler1,
                                                      timer_capture_int_handler2,
                                                      timer_capture_int_handler3,
                                                      timer_capture_int_handler4,
                                                      timer_capture_int_handler5,
                                                      timer_capture_int_handler6,
                                                      timer_capture_int_handler7,
                                                      timer_capture_int_handler8};

const uint8_t ppm_channel_map[PPM_NUM_CHANNELS] = {TIMER_OUTPUT_8, TIMER_OUTPUT_7,
                                                   TIMER_OUTPUT_6, TIMER_OUTPUT_5,
                                                   TIMER_OUTPUT_4, TIMER_OUTPUT_3,
                                                   TIMER_OUTPUT_2, TIMER_OUTPUT_1};

static void timer_ppm_int_handler(void);

static void timer_default_setup(void);
static void timer_init(timer_cap_gen_t timers[], uint8_t num,
                       void (*input_int_handlers[])(void), uint8_t num_int);
static uint32_t timer_sel_to_int_cap_event(timer_cap_gen_t *timer);
static uint32_t timer_sel_to_int_timeout_event(timer_cap_gen_t *timer);
static uint32_t timer_sel_to_int_match_event(timer_cap_gen_t *timer);
static uint64_t timer_get_total_load(timer_cap_gen_t *timer);
static void timer_calc_ps_timer_from_total(timer_cap_gen_t *timer, uint32_t *prescale,
                                           uint32_t *load, uint64_t total);
/**
 * Connects the given input to the given output. Used to pass through PWMs.
 * \param input input timer
 * \param output) output timer
 */
static void timer_connect(timer_cap_gen_t *input, timer_cap_gen_t *output);

/**
 * Disconnects the given input timer from its output
 * \param input input timer
 */
void timer_default_disconnect(uint8_t input);

/**
 * Measures or generates a pulse on the given timer.
 * \param timer if an input, reads value of pwm pulse, if output, writes pulse
 * \param pulse_width_tics number of tics that make up the pulse width of the signal.
 *                         arg not used when reading an input-timer value
 * \returns the measured or generated pulse width.
 */
static uint32_t timer_pulse(timer_cap_gen_t *timer, uint32_t pulse_width_tics);

// XXX need to change this to be a value from 1 ms to 2 ms instead of % duty
/**
 * Measures or generates a pulse on the given timer using RC standard PWM signals.
 * The pulse width value given maps from [0, UINT16_MAX] to a [05%, 10%] duty cycle
 * of the PWM signal.
 * \param timer if an input, reads value of pwm pulse, if output, writes pulse
 * \param pulse_width_RC number [0, UINT16_MAX] which mapes to [10%, 20%] duty cycle of the PWM
 *                              arg not used when reading an input-timer value
 * \returns the measured or generated pulse width.
 */
static uint16_t timer_pulse_RC(timer_cap_gen_t *timer, uint16_t pulse_width_RC);

/**
 * Reads the pulse width of the given io timer
 * \param iotimer (timer_io_t) reads value of pwm pulse on this timer
 * \return the measured or generated pulse width.
 */
static uint32_t timer_read_pulse(timer_cap_gen_t *timer);

void timer_default_init(void)
{
    timer_default_setup();
    // Initialize, only using the capture interrupts on inputs 1 - 8
    timer_init(default_timers, NUM_TIMERS,
               timer_capture_int_handlers, TIMER_INPUT_8 - TIMER_INPUT_1 + 1);

    // XXX: not strictly necessary if we implement watchdogs on the outputs
    timer_default_pulse_allpwm(timer_us_to_tics(1000));
}

static void timer_init(timer_cap_gen_t timers[], uint8_t num,
                       void (*input_int_handlers[])(void), uint8_t num_int)
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
        // XXX: This is a kinda hacky fix but it works for the defaults
        if (timers[i].gpio_pin_config) {
            GPIOPinConfigure(timers[i].gpio_pin_config);
            GPIOPinTypeTimer(gpio_ports[timers[i].gpio_port_ind], timers[i].gpio_pin);
        } else {
            GPIODirModeSet(gpio_ports[timers[i].gpio_port_ind], timers[i].gpio_pin,
                           GPIO_DIR_MODE_OUT);
            GPIOPadConfigSet(gpio_ports[timers[i].gpio_port_ind], timers[i].gpio_pin,
                             GPIO_STRENGTH_4MA, GPIO_PIN_TYPE_STD_WPU);
        }
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
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT_UP;
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
                        timer_cfg[timers[i].timer_base_ind] |= TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_ONE_SHOT_UP;
                        break;
                    default:
                        break;
                }
                break;
            default:
                while (1);
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
        sysclk_cycles_per_overflow = (   (uint64_t) timers[i].timer_val
                                       * (uint64_t) sysclk_cycles_per_second) / 1000000;
        timer_calc_ps_timer_from_total(&timers[i], &prescaler, &load, sysclk_cycles_per_overflow);

        TimerPrescaleSet(timer_bases[timers[i].timer_base_ind],
                         timer_sels[timers[i].timer_sel_ind], prescaler);
        TimerLoadSet(timer_bases[timers[i].timer_base_ind],
                     timer_sels[timers[i].timer_sel_ind], load);
        timers[i].timer_val = 0; // reset timer_val to be useful later
    }

    // Configure capture events and interrupts on the inputs and match update mode on the outputs
    uint32_t mode;
    uint8_t j = 0;
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
            if (j<num_int) {
                TimerIntRegister(timer_bases[timers[i].timer_base_ind],
                                 timer_sels[timers[i].timer_sel_ind],
                                 input_int_handlers[j++]);
            }
        } else { // generator mode (and ppm interrupt)
            switch (timers[i].capgen_mode) {
                case CAPGEN_MODE_GEN_PWM:
                    mode = TIMER_UP_LOAD_TIMEOUT | TIMER_UP_MATCH_TIMEOUT;
                    break;
                case CAPGEN_MODE_GEN_PPM:
                    mode = TIMER_UP_LOAD_IMMEDIATE | TIMER_UP_MATCH_IMMEDIATE;
                    TimerIntEnable(timer_bases[timers[i].timer_base_ind],
                                   timer_sel_to_int_timeout_event(&timers[i]));
                    IntPrioritySet(_TimerIntNumberGet(timer_bases[timers[i].timer_base_ind],
                                                      timer_sels[timers[i].timer_sel_ind]), 0x20);
                    // XXX: right now always using ppm interrupt here
                    TimerIntRegister(timer_bases[timers[i].timer_base_ind],
                                     timer_sels[timers[i].timer_sel_ind],
                                     timer_ppm_int_handler);
                    break;
                default:
                    while (1);
            }
            TimerUpdateMode(timer_bases[timers[i].timer_base_ind],
                            timer_sels[timers[i].timer_sel_ind],
                            mode);
        }
    }

    // Get outputs ready
    // XXX: handling PPM a bit weird right now
    for (i=0; i<=num; ++i) {
        switch (timers[i].capgen_mode) {
            case CAPGEN_MODE_GEN_PWM:
                timer_pulse(&timers[i], 0);
                break;
            case CAPGEN_MODE_GEN_PPM:
                TimerLoadSet(timer_bases[timers[i].timer_base_ind],
                             timer_sels[timers[i].timer_sel_ind], 1);
                TimerPrescaleSet(timer_bases[timers[i].timer_base_ind],
                                 timer_sels[timers[i].timer_sel_ind], 0);
                break;
            default:
                break;
        }
    }

    // Enable timers
    for (i=0; i<num; ++i) {
        TimerEnable(timer_bases[timers[i].timer_base_ind], timer_sels[timers[i].timer_sel_ind]);
    }
}

static void timer_connect(timer_cap_gen_t *input, timer_cap_gen_t *output)
{
    input->linked_output = output;
}

static void timer_disconnect(timer_cap_gen_t *input)
{
    input->linked_output = NULL;
}

void timer_default_connect(uint8_t input, uint8_t output)
{
    timer_connect(&default_timers[input], &default_timers[output]);
}

void timer_default_disconnect(uint8_t input)
{
    timer_disconnect(&default_timers[input]);
}

void timer_default_disconnect_all(void)
{
    uint8_t i;
    for (i=TIMER_INPUT_1; i<=TIMER_INPUT_9; ++i) timer_disconnect(&default_timers[i]);
}

static uint32_t timer_pulse(timer_cap_gen_t *timer, uint32_t pulse_width_tics)
{
    if (timer->capgen_mode & CAPGEN_MODE_CAP_MASK) { // Input timer
        return timer->timer_val;
    } else { // Output timer
        uint64_t total_load = timer_get_total_load(timer);
        uint64_t total_match;
        uint32_t prescaler_match;
        uint32_t load_match;

        if (pulse_width_tics > total_load) total_match = 0;
        else if (pulse_width_tics == 0)    total_match = total_load - 1;
        else                               total_match = total_load - pulse_width_tics;

        timer_calc_ps_timer_from_total(timer, &prescaler_match, &load_match, total_match);

        TimerPrescaleMatchSet(timer_bases[timer->timer_base_ind], timer_sels[timer->timer_sel_ind],
                              prescaler_match);
        TimerMatchSet(timer_bases[timer->timer_base_ind], timer_sels[timer->timer_sel_ind],
                      load_match);
        timer->timer_val = pulse_width_tics;
        return pulse_width_tics;
    }
}

static uint16_t timer_pulse_RC(timer_cap_gen_t *timer, uint16_t pulse_width_RC)
{
    uint64_t total_load_20 = timer_get_total_load(timer) / 20;

    uint32_t pulse_width_tics = total_load_20 + (total_load_20 * pulse_width_RC) / UINT16_MAX;
    pulse_width_tics = timer_pulse(timer, pulse_width_tics);
    if (pulse_width_tics < total_load_20) return 0;
    pulse_width_tics -= total_load_20;
    if (pulse_width_tics > total_load_20) return UINT16_MAX;
    return (UINT16_MAX * (uint64_t)pulse_width_tics) / total_load_20;
}

uint32_t timer_default_pulse(uint8_t iotimer, uint32_t pulse_width_tics)
{
    return timer_pulse(&default_timers[iotimer], pulse_width_tics);
}

uint16_t timer_default_pulse_RC(uint8_t iotimer, uint16_t pulse_width_RC)
{
    return timer_pulse_RC(&default_timers[iotimer], pulse_width_RC);
}

uint32_t timer_default_pulse_allpwm(uint32_t pulse_width_tics)
{
    uint8_t i;
    for (i=TIMER_OUTPUT_1; i<=TIMER_OUTPUT_8; ++i)
        timer_pulse(&default_timers[i], pulse_width_tics);
    return pulse_width_tics;
}

static uint32_t timer_read_pulse(timer_cap_gen_t *timer)
{
    return timer->timer_val;
}

uint32_t timer_default_read_pulse(uint8_t iotimer)
{
    return timer_read_pulse(&default_timers[iotimer]);
}

static void timer_capture_int_handler(int num)
{
    static uint64_t lastTime[TIMER_INPUT_8 + 1] = {UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                                   UINT64_MAX, UINT64_MAX, UINT64_MAX,
                                                   UINT64_MAX, UINT64_MAX};

    uint32_t timer_base = timer_bases[default_timers[num].timer_base_ind];
    uint32_t timer_sel = timer_sels[default_timers[num].timer_sel_ind];
    uint32_t timer_int = timer_sel_to_int_cap_event(&default_timers[num]);
    uint8_t gpio_level;
    uint64_t time;
    uint64_t total_load;

    TimerIntClear(timer_base, timer_int);
    gpio_level = GPIOPinRead(gpio_ports[default_timers[num].gpio_port_ind],
                             default_timers[num].gpio_pin);
    time = TimerValueGet(timer_base, timer_sel);
    total_load = timer_default_get_total_load(num);

    if (lastTime[num] < UINT64_MAX) {
        uint32_t delta;
        if(lastTime[num] < time) delta = lastTime[num] + total_load - time;
        else delta = lastTime[num] - time;

        // GPIO level low means we have caught a falling edge
        if (gpio_level == 0) {
            default_timers[num].timer_val = delta;
            if (default_timers[num].linked_output != NULL)
                timer_pulse(default_timers[num].linked_output, delta);
        }
    }
    lastTime[num] = time;
}

static void timer_ppm_int_handler(void)
{
#define START_PULSE 0
#define END_PULSE 1

    uint32_t timer_base = timer_bases[default_timers[TIMER_OUTPUT_9].timer_base_ind];
    uint32_t timer_sel  = timer_sels[default_timers[TIMER_OUTPUT_9].timer_sel_ind];
    uint32_t gpio_port  = gpio_ports[default_timers[TIMER_OUTPUT_9].gpio_port_ind];
    uint8_t  gpio_pin   = default_timers[TIMER_OUTPUT_9].gpio_pin;

    TimerIntClear(timer_base, timer_sel_to_int_timeout_event(&default_timers[TIMER_OUTPUT_9]));

    static uint8_t pulse_state = START_PULSE;
    static uint8_t channel_num = 0;
    static uint32_t ppm_total_period_left_us = PPM_TOTAL_PERIOD_US;
    uint32_t tics;
    uint32_t us;

    switch(pulse_state)
    {
        case START_PULSE:
            us = PPM_START_PULSE_US;
            tics = timer_us_to_tics(us);
            ppm_total_period_left_us -= us;
            GPIOPinWrite(gpio_port, gpio_pin, 0);
            pulse_state = END_PULSE;
            break;

        case END_PULSE:
            if(channel_num < PPM_NUM_CHANNELS)
            {
                tics = timer_pwm_to_ppm_RC_convention(
                        default_timers[ppm_channel_map[channel_num]].timer_val);
                us = timer_tics_to_us(tics);
                ppm_total_period_left_us -= us;
                ++channel_num;
            }
            else
            {
                us = ppm_total_period_left_us;
                tics = timer_us_to_tics(us);
                ppm_total_period_left_us = PPM_TOTAL_PERIOD_US;
                channel_num = 0;
            }
            GPIOPinWrite(gpio_port, gpio_pin, gpio_pin);
            pulse_state = START_PULSE;
            break;

        default:
            break;
    }

    uint32_t prescale;
    uint32_t load;
    timer_default_calc_ps_timer_from_total(TIMER_OUTPUT_9, &prescale, &load, tics);
    TimerPrescaleSet(timer_base, timer_sel, prescale);
    TimerLoadSet(timer_base, timer_sel, load);
    TimerEnable(timer_base, timer_sel);

#undef END_PULSE
#undef START_PULSE
}

// XXX: make more functions like this to clean up the initializer
static uint32_t timer_sel_to_int_cap_event(timer_cap_gen_t *timer)
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

static uint32_t timer_sel_to_int_timeout_event(timer_cap_gen_t *timer)
{
    switch (timer->timer_sel_ind) {
        case TIMERA:
            return TIMER_TIMA_TIMEOUT;
        case TIMERB:
            return TIMER_TIMB_TIMEOUT;
        default:
            return 0;
    }
}

static uint64_t timer_get_total_load(timer_cap_gen_t *timer)
{
    uint32_t prescaler = TimerPrescaleGet(timer_bases[timer->timer_base_ind],
                                          timer_sels[timer->timer_sel_ind]);
    uint32_t load = TimerLoadGet(timer_bases[timer->timer_base_ind],
                                 timer_sels[timer->timer_sel_ind]);

    if (timer->timer_base_ind <= TIMER5) { // 16 bit timer with 8 bit prescaler
        return ((prescaler << 16) & 0x00FF0000) | (load & 0x0000FFFF);
    } else { // 32 bit timer with 16 bit prescaler
        return ((((uint64_t)prescaler) << 32) & 0x0000FFFF00000000) | (load & 0x00000000FFFFFFFF);
    }
}

uint64_t timer_default_get_total_load(uint8_t iotimer)
{
    return timer_get_total_load(&default_timers[iotimer]);
}

static void timer_calc_ps_timer_from_total(timer_cap_gen_t *timer, uint32_t *prescale,
                                           uint32_t *load, uint64_t total)
{
    if (timer->timer_base_ind <= TIMER5) { // 16 bit timer with 8 bit prescaler
        *prescale = (total >> 16) & 0x0000FFFF;
        if (*prescale > 0x000000FF) while (1);
        *load = total & 0x0000FFFF;
    } else { // 32 bit timer with 16 bit prescaler
        *prescale = (total >> 32) & 0x00000000FFFFFFFF;
        if (*prescale > 0x0000FFFF) while (1);
        *load = total & 0x00000000FFFFFFFF;
    }
}

void timer_default_calc_ps_timer_from_total(uint8_t iotimer, uint32_t *prescale,
                                            uint32_t *load, uint64_t total)
{
    timer_calc_ps_timer_from_total(&default_timers[iotimer], prescale, load, total);
}

uint32_t timer_us_to_tics(uint32_t us)
{
    return ((uint64_t)us * SysCtlClockGet()) / 1000000;
}

uint32_t timer_tics_to_us(uint32_t tics)
{
    return ((uint64_t)tics * 1000000) / SysCtlClockGet();
}

uint32_t timer_pwm_to_ppm_RC_convention(uint32_t pwm_tics)
{
    // need to map [1, 2] ms to [0.7, 1.7] ms
    uint32_t pwm_us = timer_tics_to_us(pwm_tics);
    uint32_t ppm_us;
    if (pwm_us < 1000) ppm_us = 700;
    else if (pwm_us > 2000) ppm_us = 1700;
    else ppm_us = pwm_us - 300;
    return timer_us_to_tics(ppm_us);
}

static void timer_default_setup(void)
{
    default_timers[TIMER_INPUT_1]  = (timer_cap_gen_t){GPIO_PD3_WT3CCP1, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_1],
                                                       GPIO_PD, GPIO_PIN_3,
                                                       WTIMER3, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_2]  = (timer_cap_gen_t){GPIO_PD2_WT3CCP0, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_2],
                                                       GPIO_PD, GPIO_PIN_2,
                                                       WTIMER3, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_3]  = (timer_cap_gen_t){GPIO_PD1_WT2CCP1, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_3],
                                                       GPIO_PD, GPIO_PIN_1,
                                                       WTIMER2, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_4]  = (timer_cap_gen_t){GPIO_PB6_T0CCP0, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_4],
                                                       GPIO_PB, GPIO_PIN_6,
                                                       TIMER0, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_5]  = (timer_cap_gen_t){GPIO_PB4_T1CCP0, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_5],
                                                       GPIO_PB, GPIO_PIN_4,
                                                       TIMER1, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_6]  = (timer_cap_gen_t){GPIO_PB5_T1CCP1, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_6],
                                                       GPIO_PB, GPIO_PIN_5,
                                                       TIMER1, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_7]  = (timer_cap_gen_t){GPIO_PD6_WT5CCP0, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_7],
                                                       GPIO_PD, GPIO_PIN_6,
                                                       WTIMER5, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_8]  = (timer_cap_gen_t){GPIO_PB3_T3CCP1, OVERFLOW_60MS,
                                                       &default_timers[TIMER_OUTPUT_8],
                                                       GPIO_PB, GPIO_PIN_3,
                                                       TIMER3, TIMERB, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_INPUT_9]  = (timer_cap_gen_t){GPIO_PB2_T3CCP0, OVERFLOW_60MS, NULL,
                                                       GPIO_PB, GPIO_PIN_2,
                                                       TIMER3, TIMERA, CAPGEN_MODE_CAP_RF_EDGE};

    default_timers[TIMER_OUTPUT_1] = (timer_cap_gen_t){GPIO_PG1_T4CCP1, OVERFLOW_20MS, NULL,
                                                       GPIO_PG, GPIO_PIN_1,
                                                       TIMER4, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_2] = (timer_cap_gen_t){GPIO_PG2_T5CCP0, OVERFLOW_20MS, NULL,
                                                       GPIO_PG, GPIO_PIN_2,
                                                       TIMER5, TIMERA, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_3] = (timer_cap_gen_t){GPIO_PG3_T5CCP1, OVERFLOW_20MS, NULL,
                                                       GPIO_PG, GPIO_PIN_3,
                                                       TIMER5, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_4] = (timer_cap_gen_t){GPIO_PC4_WT0CCP0, OVERFLOW_20MS, NULL,
                                                       GPIO_PC, GPIO_PIN_4,
                                                       WTIMER0, TIMERA, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_5] = (timer_cap_gen_t){GPIO_PC5_WT0CCP1, OVERFLOW_20MS, NULL,
                                                       GPIO_PC, GPIO_PIN_5,
                                                       WTIMER0, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_6] = (timer_cap_gen_t){GPIO_PC6_WT1CCP0, OVERFLOW_20MS, NULL,
                                                       GPIO_PC, GPIO_PIN_6,
                                                       WTIMER1, TIMERA, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_7] = (timer_cap_gen_t){GPIO_PC7_WT1CCP1, OVERFLOW_20MS, NULL,
                                                       GPIO_PC, GPIO_PIN_7,
                                                       WTIMER1, TIMERB, CAPGEN_MODE_GEN_PWM};

    default_timers[TIMER_OUTPUT_8] = (timer_cap_gen_t){GPIO_PB7_T0CCP1, OVERFLOW_20MS, NULL,
                                                       GPIO_PB, GPIO_PIN_7,
                                                       TIMER0, TIMERB, CAPGEN_MODE_GEN_PWM};

//    default_timers[TIMER_OUTPUT_9] = (timer_cap_gen_t){GPIO_PG0_T4CCP0, OVERFLOW_PPM,
    default_timers[TIMER_OUTPUT_9] = (timer_cap_gen_t){0, OVERFLOW_PPM, NULL,
                                                       GPIO_PG, GPIO_PIN_0,
                                                       TIMER4, TIMERA, CAPGEN_MODE_GEN_PPM};
}

static void timer_capture_int_handler1(void)
{
    timer_capture_int_handler(TIMER_INPUT_1);
}

static void timer_capture_int_handler2(void)
{
    timer_capture_int_handler(TIMER_INPUT_2);
}

static void timer_capture_int_handler3(void)
{
    timer_capture_int_handler(TIMER_INPUT_3);
}

static void timer_capture_int_handler4(void)
{
    timer_capture_int_handler(TIMER_INPUT_4);
}

static void timer_capture_int_handler5(void)
{
    timer_capture_int_handler(TIMER_INPUT_5);
}

static void timer_capture_int_handler6(void)
{
    timer_capture_int_handler(TIMER_INPUT_6);
}

static void timer_capture_int_handler7(void)
{
    timer_capture_int_handler(TIMER_INPUT_7);
}

static void timer_capture_int_handler8(void)
{
    timer_capture_int_handler(TIMER_INPUT_8);
}
