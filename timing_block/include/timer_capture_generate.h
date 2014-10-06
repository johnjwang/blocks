/*
 * timer_capture_generate.h
 *
 *  Created on: Oct 2, 2014
 *      Author: Isaac
 */

#ifndef TIMER_CAPTURE_GENERATE_H_
#define TIMER_CAPTURE_GENERATE_H_

// Input-Output port numbers
typedef enum _timer_io_t{
    TIMER_INPUT_1,  /*D3*/
    TIMER_INPUT_2,  /*D2*/
    TIMER_INPUT_3,  /*D1*/
    TIMER_INPUT_4,  /*B6*/
    TIMER_INPUT_5,  /*B4*/
    TIMER_INPUT_6,  /*B5*/
    TIMER_INPUT_7,  /*D6*/
    TIMER_INPUT_8,  /*B3*/
    TIMER_INPUT_9,  /*B2*/
    TIMER_OUTPUT_1, /*G1*/
    TIMER_OUTPUT_2, /*G2*/
    TIMER_OUTPUT_3, /*G3*/
    TIMER_OUTPUT_4, /*C4*/
    TIMER_OUTPUT_5, /*C5*/
    TIMER_OUTPUT_6, /*C6*/
    TIMER_OUTPUT_7, /*C7*/
    TIMER_OUTPUT_8, /*B7*/
    TIMER_OUTPUT_9, /*G0*/
    NUM_TIMERS
} timer_io_t;

// Timer Base index values
typedef enum _timer_base_indices_t {
     TIMER0,
     TIMER1,
     TIMER2,
     TIMER3,
     TIMER4,
     TIMER5,
    WTIMER0,
    WTIMER1,
    WTIMER2,
    WTIMER3,
    WTIMER4,
    WTIMER5,
    NUM_TIMER_BASES
} timer_base_indices_t;


// Timer selection index values
typedef enum _timer_select_indices_t {
    TIMERA,
    TIMERB,
    NUM_TIMER_SELECTIONS
} timer_select_indices_t;

// GPIO port index values
typedef enum _gpio_port_indices_t {
    GPIO_PA,
    GPIO_PB,
    GPIO_PC,
    GPIO_PD,
    GPIO_PE,
    GPIO_PF,
    GPIO_PG,
    NUM_GPIO_PORTS
} gpio_port_indices_t;

// Timer Overflow Settings
#define OVERFLOW_60MS 60000
#define OVERFLOW_20MS 20000

// Capture / Generate modes, least significant 4 bits is a capture code
// most significant 4 bits is a generate code (can't be both)
#define CAPGEN_MODE_CAP_MASK    0x0F
#define CAPGEN_MODE_CAP_R_EDGE  0x01
#define CAPGEN_MODE_CAP_F_EDGE  0x02
#define CAPGEN_MODE_CAP_RF_EDGE 0x03
#define CAPGEN_MODE_GEN_MASK    0xF0
#define CAPGEN_MODE_GEN_PWM     0x10
#define CAPGEN_MODE_GEN_PPM     0x20

// Note, these are initialized with initializer syntax in default setup
// so don't change the order of the data
typedef struct _timer_cap_gen_t
{
    uint32_t gpio_pin_config;      // GPIO_Pcx_TxCCPx, GPIO_Pcx_WTxCCPx

    uint32_t us_per_overflow;      // for setting prescaler and load, times clock to reset in x ms

    uint8_t  gpio_port_ind;        // GPIO_Pc
    uint8_t  gpio_pin;             // GPIO_PIN_x

    uint8_t  timer_base_ind;       // TIMERx, WTIMERx
    uint8_t  timer_sel_ind;        // TIMERA, TIMERB

    uint8_t  capgen_mode;          // CAPGEN_MODE_?????

} timer_cap_gen_t;

void timer_default_setup(void);
void timer_default_init(void);
void timer_init(timer_cap_gen_t timers[], uint8_t num);

void timer_capture_generate_init(void);
void timer_generate_pulse(uint32_t pulse_width);
void timer_generate_pulse_percent(float percent);
void timer_capture_generate_start(void);

#endif /* TIMER_CAPTURE_GENERATE_H_ */
