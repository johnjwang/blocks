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

uint32_t sys_clk_cycles_per_second;
uint32_t ms_per_capture_period = 60;
uint32_t ms_per_pwm_period = 20;
uint32_t cycles_per_capture_period;
uint32_t cycles_per_pwm_period;
uint32_t timer_prescaler;
uint32_t timer_generate_load;

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
        if (timer_cfg) TimerConfigure(timer_bases[i], timer_cfg[i]);
    }

    // Calculate load and prescale
    uint32_t sysclk_cycles_per_second = SysCtlClockGet();
    uint64_t sysclk_cycles_per_overflow;
    uint32_t prescaler;
    uint32_t load;
    for (i=0; i<num; ++i) {
         sysclk_cycles_per_overflow = ((uint64_t)timers[i].us_per_overflow *
                                       (uint64_t)sys_clk_cycles_per_second) / 1000000;
         if (timers[i].timer_base_ind <= TIMER5) { // 16 bit timer with 8 bit prescaler
             prescaler = (sysclk_cycles_per_overflow & 0xFFFF0000) >> 16;
             load      = (sysclk_cycles_per_overflow & 0x0000FFFF);
         } else { // 32 bit timer with 16 bit prescaler
             prescaler = (sysclk_cycles_per_overflow & 0xFFFFFFFF00000000) >> 32;
             load      = (sysclk_cycles_per_overflow & 0x00000000FFFFFFFF);
         }
         TimerPrescaleSet(timer_bases[timers[i].timer_base_ind], timer_sels[timers[i].timer_sel_ind],
                          prescaler);
         TimerLoadSet(timer_bases[timers[i].timer_base_ind], timer_sels[timers[i].timer_sel_ind],
                      load);
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
                    break;
            }
            TimerControlEvent(timer_bases[timers[i].timer_base_ind],
                              timer_sels[timers[i].timer_sel_ind],
                              mode);

            switch (timers[i].timer_sel_ind) {
                case TIMERA:
                    mode = TIMER_CAPA_EVENT;
                    break;
                case TIMERB:
                    mode = TIMER_CAPB_EVENT;
                    break;
                default:
                    while (1);
                    break;
            }
            TimerIntEnable(timer_bases[timers[i].timer_base_ind], mode);
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
                    break;
            }
            TimerUpdateMode(timer_bases[timers[i].timer_base_ind],
                                    timer_sels[timers[i].timer_sel_ind],
                                    mode);
        }
    }
}

void timer_capture_generate_init(void)
{
	// GPIO port enable
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOG);

	// Capture setup							   // CCP0      |  CCP1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // Input  4  |  Output 8
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);  // Input  5  |  Input  6
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);  // Input  9  |  Input  8
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER4);  // Output 9  |  Output 1
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER5);  // Output 2  |  Output 3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER0); // Output 4  |  Output 5
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER1); // Output 6  |  Output 7
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER2); //           |  Input  3
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER3); // Input  1  |  Input  2
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WTIMER5); // Input  7  |

	// GPIO pin enable
	GPIOPinConfigure(GPIO_PD3_WT3CCP1); // Input 1
	GPIOPinConfigure(GPIO_PD2_WT3CCP0); // Input 2
	GPIOPinConfigure(GPIO_PD1_WT2CCP1); // Input 3
	GPIOPinConfigure(GPIO_PB6_T0CCP0);  // Input 4
	GPIOPinConfigure(GPIO_PB4_T1CCP0);  // Input 5
	GPIOPinConfigure(GPIO_PB5_T1CCP1);  // Input 6
	GPIOPinConfigure(GPIO_PD6_WT5CCP0); // Input 7
	GPIOPinConfigure(GPIO_PB3_T3CCP1);  // Input 8
	GPIOPinConfigure(GPIO_PB2_T3CCP0);  // Input 9
	GPIOPinConfigure(GPIO_PG1_T4CCP1);  // Output 1
	GPIOPinConfigure(GPIO_PG2_T5CCP0);  // Output 2
	GPIOPinConfigure(GPIO_PG3_T5CCP1);  // Output 3
	GPIOPinConfigure(GPIO_PC4_WT0CCP0); // Output 4
	GPIOPinConfigure(GPIO_PC5_WT0CCP1); // Output 5
	GPIOPinConfigure(GPIO_PC6_WT1CCP0); // Output 6
	GPIOPinConfigure(GPIO_PC7_WT1CCP1); // Output 7
	GPIOPinConfigure(GPIO_PB7_T0CCP1);  // Output 8
	GPIOPinConfigure(GPIO_PG0_T4CCP0);  // Output 9

	// GPIO pin type
	GPIOPinTypeTimer(GPIO_PORTB_BASE,                           GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	GPIOPinTypeTimer(GPIO_PORTC_BASE,                                                     GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
	GPIOPinTypeTimer(GPIO_PORTD_BASE,              GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |                           GPIO_PIN_6             );
	GPIOPinTypeTimer(GPIO_PORTG_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3                                                    );

    GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_3, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_1, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_6, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_4, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_6, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD_WPU);

	// Timer Configuration
	TimerConfigure(TIMER0_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_PWM     );
	TimerConfigure(TIMER1_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_CAP_TIME);
	TimerConfigure(TIMER3_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_CAP_TIME);
	TimerConfigure(TIMER4_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(TIMER5_BASE,  TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(WTIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(WTIMER1_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM      | TIMER_CFG_B_PWM     );
	TimerConfigure(WTIMER2_BASE, TIMER_CFG_SPLIT_PAIR |                        TIMER_CFG_B_CAP_TIME);
	TimerConfigure(WTIMER3_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME | TIMER_CFG_B_CAP_TIME);
	TimerConfigure(WTIMER5_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_CAP_TIME                       );

	// Period and Prescalar Calculation
	sys_clk_cycles_per_second = SysCtlClockGet();
	cycles_per_capture_period = ((uint64_t)ms_per_capture_period * (uint64_t)sys_clk_cycles_per_second) / 1000;
	cycles_per_pwm_period     = ((uint64_t)ms_per_pwm_period * (uint64_t)sys_clk_cycles_per_second) / 1000;

	// Set Prescale Value
	uint32_t pwm_prescaler     = (cycles_per_pwm_period     & 0xFFFF0000) >> 16;
	uint32_t capture_prescaler = (cycles_per_capture_period & 0xFFFF0000) >> 16;
	if (pwm_prescaler > 255) while(1);
	if (capture_prescaler > 255) while(1);
    TimerPrescaleSet(TIMER0_BASE,  TIMER_A, capture_prescaler); // input
    TimerPrescaleSet(TIMER0_BASE,  TIMER_B, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER1_BASE,  TIMER_A, capture_prescaler); // input
    TimerPrescaleSet(TIMER1_BASE,  TIMER_B, capture_prescaler); // input
    TimerPrescaleSet(TIMER3_BASE,  TIMER_A, capture_prescaler); // input
    TimerPrescaleSet(TIMER3_BASE,  TIMER_B, capture_prescaler); // input
    TimerPrescaleSet(TIMER4_BASE,  TIMER_A, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER4_BASE,  TIMER_B, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER5_BASE,  TIMER_A, pwm_prescaler      ); // output
    TimerPrescaleSet(TIMER5_BASE,  TIMER_B, pwm_prescaler      ); // output
//    TimerPrescaleSet(WTIMER0_BASE, TIMER_A, 0                  ); // output
//    TimerPrescaleSet(WTIMER0_BASE, TIMER_B, 0                  ); // output
//    TimerPrescaleSet(WTIMER1_BASE, TIMER_A, 0                  ); // output
//    TimerPrescaleSet(WTIMER1_BASE, TIMER_B, 0                  ); // output
//    TimerPrescaleSet(WTIMER2_BASE, TIMER_B, 0                  ); // input
//    TimerPrescaleSet(WTIMER3_BASE, TIMER_A, 0                  ); // input
//    TimerPrescaleSet(WTIMER3_BASE, TIMER_B, 0                  ); // input
//    TimerPrescaleSet(WTIMER5_BASE, TIMER_A, 0                  ); // input

	// Set Load Value
    uint32_t pwm_load     = cycles_per_pwm_period     & 0xFFFF;
    uint32_t capture_load = cycles_per_capture_period & 0xFFFF;
	TimerLoadSet(TIMER0_BASE,  TIMER_A, capture_load);
	TimerLoadSet(TIMER0_BASE,  TIMER_B, pwm_load                  );
	TimerLoadSet(TIMER1_BASE,  TIMER_A, capture_load              );
	TimerLoadSet(TIMER1_BASE,  TIMER_B, capture_load              );
	TimerLoadSet(TIMER3_BASE,  TIMER_A, capture_load              );
	TimerLoadSet(TIMER3_BASE,  TIMER_B, capture_load              );
	TimerLoadSet(TIMER4_BASE,  TIMER_A, pwm_load                  );
	TimerLoadSet(TIMER4_BASE,  TIMER_B, pwm_load                  );
	TimerLoadSet(TIMER5_BASE,  TIMER_A, pwm_load                  );
	TimerLoadSet(TIMER5_BASE,  TIMER_B, pwm_load                  );
  	TimerLoadSet(WTIMER0_BASE, TIMER_A, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER0_BASE, TIMER_B, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER1_BASE, TIMER_A, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER1_BASE, TIMER_B, cycles_per_pwm_period     );
  	TimerLoadSet(WTIMER2_BASE, TIMER_B, cycles_per_capture_period );
  	TimerLoadSet(WTIMER3_BASE, TIMER_A, cycles_per_capture_period );
  	TimerLoadSet(WTIMER3_BASE, TIMER_B, cycles_per_capture_period );
  	TimerLoadSet(WTIMER5_BASE, TIMER_A, cycles_per_capture_period );

	// Set PWM Generators to Update Match Value After Overflow
	TimerUpdateMode(TIMER0_BASE,  TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER4_BASE,  TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER4_BASE,  TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER5_BASE,  TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(TIMER5_BASE,  TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER0_BASE, TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER0_BASE, TIMER_B, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER1_BASE, TIMER_A, TIMER_UP_MATCH_TIMEOUT);
	TimerUpdateMode(WTIMER1_BASE, TIMER_B, TIMER_UP_MATCH_TIMEOUT);

    // Set PWM to 0 Duty Cycle
	uint32_t duty_cycle = cycles_per_pwm_period / 2; // 50% duty cycle
	timer_generate_pulse(duty_cycle);

	// Configure Capture Event
	TimerControlEvent(TIMER0_BASE,  TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER1_BASE,  TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER1_BASE,  TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER3_BASE,  TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(TIMER3_BASE,  TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER2_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER3_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER3_BASE, TIMER_B, TIMER_EVENT_BOTH_EDGES);
	TimerControlEvent(WTIMER5_BASE, TIMER_A, TIMER_EVENT_BOTH_EDGES);

	// Enable Timer Interrupts
	TimerIntEnable(TIMER0_BASE,  TIMER_CAPA_EVENT);
    TimerIntEnable(TIMER1_BASE,  TIMER_CAPA_EVENT);
    TimerIntEnable(TIMER1_BASE,  TIMER_CAPB_EVENT);
    TimerIntEnable(TIMER3_BASE,  TIMER_CAPA_EVENT);
    TimerIntEnable(TIMER3_BASE,  TIMER_CAPB_EVENT);
    TimerIntEnable(WTIMER2_BASE, TIMER_CAPB_EVENT);
    TimerIntEnable(WTIMER3_BASE, TIMER_CAPA_EVENT);
    TimerIntEnable(WTIMER3_BASE, TIMER_CAPB_EVENT);
    TimerIntEnable(WTIMER5_BASE, TIMER_CAPA_EVENT);

    // Prioritize Capture Interrupt
    IntPrioritySet(INT_TIMER0A, 0x20);
    IntPrioritySet(INT_TIMER1A, 0x20);
    IntPrioritySet(INT_TIMER1B, 0x20);
    IntPrioritySet(INT_TIMER3A, 0x20);
    IntPrioritySet(INT_TIMER3B, 0x20);
    IntPrioritySet(INT_WTIMER2B, 0x20);
    IntPrioritySet(INT_WTIMER3A, 0x20);
    IntPrioritySet(INT_WTIMER3B, 0x20);
    IntPrioritySet(INT_WTIMER5A, 0x20);

	// Register Capture Interrupt
	TimerIntRegister(TIMER0_BASE,  TIMER_A, timer_capture_int_handlers[0]);
	TimerIntRegister(TIMER1_BASE,  TIMER_A, timer_capture_int_handlers[1]);
	TimerIntRegister(TIMER1_BASE,  TIMER_B, timer_capture_int_handlers[2]);
	TimerIntRegister(TIMER3_BASE,  TIMER_A, timer_capture_int_handlers[3]);
	TimerIntRegister(TIMER3_BASE,  TIMER_B, timer_capture_int_handlers[4]);
	TimerIntRegister(WTIMER2_BASE, TIMER_B, timer_capture_int_handlers[5]);
	TimerIntRegister(WTIMER3_BASE, TIMER_A, timer_capture_int_handlers[6]);
	TimerIntRegister(WTIMER3_BASE, TIMER_B, timer_capture_int_handlers[7]);
	TimerIntRegister(WTIMER5_BASE, TIMER_A, timer_capture_int_handlers[8]);

	// Enable Interrupt
	// XXX: shouldn't need this, the register call does this
    IntEnable(INT_TIMER0A);
    IntEnable(INT_TIMER1A);
    IntEnable(INT_TIMER1B);
    IntEnable(INT_TIMER3A);
    IntEnable(INT_TIMER3B);
    IntEnable(INT_WTIMER2B);
    IntEnable(INT_WTIMER3A);
    IntEnable(INT_WTIMER3B);
    IntEnable(INT_WTIMER5A);
}

void timer_capture_generate_start(void)
{
    // Enable Timer Captures
    TimerEnable(TIMER0_BASE,  TIMER_A);
    TimerEnable(TIMER1_BASE,  TIMER_A);
    TimerEnable(TIMER1_BASE,  TIMER_B);
    TimerEnable(TIMER3_BASE,  TIMER_A);
    TimerEnable(TIMER3_BASE,  TIMER_B);
    TimerEnable(WTIMER2_BASE, TIMER_B);
    TimerEnable(WTIMER3_BASE, TIMER_A);
    TimerEnable(WTIMER3_BASE, TIMER_B);
    TimerEnable(WTIMER5_BASE, TIMER_A);

    // Enable Timer Generators
    TimerEnable(TIMER0_BASE,  TIMER_B);
    TimerEnable(TIMER4_BASE,  TIMER_A);
    TimerEnable(TIMER4_BASE,  TIMER_B);
    TimerEnable(TIMER5_BASE,  TIMER_A);
    TimerEnable(TIMER5_BASE,  TIMER_B);
    TimerEnable(WTIMER0_BASE, TIMER_A);
    TimerEnable(WTIMER0_BASE, TIMER_B);
    TimerEnable(WTIMER1_BASE, TIMER_A);
    TimerEnable(WTIMER1_BASE, TIMER_B);
}

void timer_generate_pulse_percent(float percent)
{
    timer_generate_pulse(percent * cycles_per_pwm_period);
}

void timer_generate_pulse(uint32_t pulse_width)
{
	uint32_t match_set;
	if (pulse_width > cycles_per_pwm_period) match_set = 0;
	else if(pulse_width == 0) match_set = cycles_per_pwm_period - 1;
	else match_set = cycles_per_pwm_period - pulse_width;

    uint32_t match_set_upper_8  = (match_set & 0xff0000) >> 16;
    uint32_t match_set_lower_16 =  match_set & 0xffff;
    TimerPrescaleMatchSet(TIMER0_BASE,  TIMER_B, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER4_BASE,  TIMER_A, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER4_BASE,  TIMER_B, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER5_BASE,  TIMER_A, match_set_upper_8);
    TimerPrescaleMatchSet(TIMER5_BASE,  TIMER_B, match_set_upper_8);
    TimerMatchSet(TIMER0_BASE,  TIMER_B, match_set_lower_16);
    TimerMatchSet(TIMER4_BASE,  TIMER_A, match_set_lower_16);
    TimerMatchSet(TIMER4_BASE,  TIMER_B, match_set_lower_16);
    TimerMatchSet(TIMER5_BASE,  TIMER_A, match_set_lower_16);
    TimerMatchSet(TIMER5_BASE,  TIMER_B, match_set_lower_16);
    TimerMatchSet(WTIMER0_BASE, TIMER_A, match_set);
    TimerMatchSet(WTIMER0_BASE, TIMER_B, match_set);
    TimerMatchSet(WTIMER1_BASE, TIMER_A, match_set);
    TimerMatchSet(WTIMER1_BASE, TIMER_B, match_set);

    uint8_t msg[20] = {0};
    usb_write(msg, snprintf((char*)msg, 20, "%lu\r\n", match_set));
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
    static long lastTime = 0;
    uint64_t time;
    switch(num)
    {
        case 0:
            TimerIntClear(TIMER0_BASE, TIMER_CAPA_EVENT);
//            time = ((TimerPrescaleGet(TIMER0_BASE, TIMER_A) << 16) | TimerValueGet(TIMER0_BASE, TIMER_A))/80;
            time = TimerValueGet(TIMER0_BASE, TIMER_A);
            break;
        case 1:
            TimerIntClear(TIMER1_BASE, TIMER_CAPA_EVENT);
//            time =  ((TimerPrescaleGet(TIMER1_BASE, TIMER_A) << 16) | TimerValueGet(TIMER1_BASE, TIMER_A))/80;
            time = TimerValueGet(TIMER1_BASE, TIMER_A);
            break;
        case 2:
            TimerIntClear(TIMER1_BASE, TIMER_CAPB_EVENT);
//            time = ((TimerPrescaleGet(TIMER1_BASE, TIMER_B) << 16) | TimerValueGet(TIMER1_BASE, TIMER_B))/80;
            time = TimerValueGet(TIMER1_BASE, TIMER_B);
            break;
        case 3:
            TimerIntClear(TIMER3_BASE, TIMER_CAPA_EVENT);
//            time = ((TimerPrescaleGet(TIMER3_BASE, TIMER_A) << 16) | TimerValueGet(TIMER3_BASE, TIMER_A))/80;
            time = TimerValueGet(TIMER3_BASE, TIMER_A);
            break;
        case 4:
            TimerIntClear(TIMER3_BASE, TIMER_CAPB_EVENT);
//            time = ((TimerPrescaleGet(TIMER3_BASE, TIMER_B) << 16) | TimerValueGet(TIMER3_BASE, TIMER_B))/80;
            time = TimerValueGet(TIMER3_BASE, TIMER_B);
            break;
        case 5:
            TimerIntClear(WTIMER2_BASE, TIMER_CAPB_EVENT);
//            time = TimerValueGet(WTIMER2_BASE, TIMER_B)/80;
            time = TimerValueGet(WTIMER2_BASE, TIMER_B);
            break;
        case 6:
            TimerIntClear(WTIMER3_BASE, TIMER_CAPA_EVENT);
//            time = TimerValueGet(WTIMER3_BASE, TIMER_A)/80;
            time = TimerValueGet(WTIMER3_BASE, TIMER_A);
            break;
        case 7:
            TimerIntClear(WTIMER3_BASE, TIMER_CAPB_EVENT);
//            time = TimerValueGet(WTIMER3_BASE, TIMER_B)/80;
            time = TimerValueGet(WTIMER3_BASE, TIMER_B);
            break;
        case 8:
            TimerIntClear(WTIMER5_BASE, TIMER_CAPA_EVENT);
//            time = TimerValueGet(WTIMER5_BASE, TIMER_A)/80;
            time = TimerValueGet(WTIMER5_BASE, TIMER_A);
            break;
    }
    uint32_t delta;
    if(lastTime < time) delta = lastTime + cycles_per_capture_period - time;
    else delta = lastTime - time;
    usb_write(msg, snprintf((char*)msg, 30, "%ld\r\n", delta/80));
    lastTime = time;
}
