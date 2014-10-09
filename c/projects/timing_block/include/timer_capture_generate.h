/*
 * timer_capture_generate.h
 *
 *  Created on: Oct 2, 2014
 *      Author: Isaac
 */

#ifndef TIMER_CAPTURE_GENERATE_H_
#define TIMER_CAPTURE_GENERATE_H_

// Input-Output port numbers XXX: (position comments are for v1 block)
typedef enum _timer_io_t{
    TIMER_INPUT_1,  /*D3*/ /*next to 5V*/
    TIMER_INPUT_2,  /*D2*/
    TIMER_INPUT_3,  /*D1*/
    TIMER_INPUT_4,  /*B6*/
    TIMER_INPUT_5,  /*B4*/
    TIMER_INPUT_6,  /*B5*/
    TIMER_INPUT_7,  /*D6*/
    TIMER_INPUT_8,  /*B3*/ /*next to gnd*/
    TIMER_INPUT_9,  /*B2*/ /*in 3 pin jst*/
    TIMER_OUTPUT_1, /*G1*/ /*next to 5V*/
    TIMER_OUTPUT_2, /*G2*/
    TIMER_OUTPUT_3, /*G3*/
    TIMER_OUTPUT_4, /*C4*/
    TIMER_OUTPUT_5, /*C5*/
    TIMER_OUTPUT_6, /*C6*/
    TIMER_OUTPUT_7, /*C7*/
    TIMER_OUTPUT_8, /*B7*/ /*next to gnd*/
    TIMER_OUTPUT_9, /*G0*/ /*in 3 pin jst*/
    NUM_TIMERS
} timer_io_t;

// Timer Base index values
typedef enum _timer_base_indices_t {
// Assignment:     TimerA         TimerB
     TIMER0,   /*    B6    */ /*    B7    */
     TIMER1,   /*    B4    */ /*    B5    */
     TIMER2,   /*sigtimeout*/ /*          */
     TIMER3,   /*    B2    */ /*    B3    */
     TIMER4,   /*    G0    */ /*    G1    */
     TIMER5,   /*    G2    */ /*    G3    */
    WTIMER0,   /*    C4    */ /*    C5    */
    WTIMER1,   /*    C6    */ /*    C7    */
    WTIMER2,   /*          */ /*    D1    */
    WTIMER3,   /*    D2    */ /*    D3    */
    WTIMER4,   /*   leds   */ /* timestamp*/
    WTIMER5,   /*    D6    */ /*          */
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

#define OVERFLOW_PWM_CAP 60000
//#define OVERFLOW_PWM_GEN 20000
#define OVERFLOW_PWM_GEN  2500
#define OVERFLOW_PPM 1
#define OVERFLOW_SIGTIMEOUT 20000
// XXX: make throttle enums like 1000 us and 2000 us for RC mode

// Timer Lifetime Settings
#define TIMER_LIFETIMES_10 10
#define TIMER_LIFETIMES_INF -1

// PPM settings
#define PPM_NUM_CHANNELS 8
#define PPM_START_PULSE_US 300
#define PPM_TOTAL_PERIOD_US 22500

// list inputs in the order they should be muxed into the ppm signal
extern const uint8_t ppm_channel_map[];

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
typedef struct timer_cap_gen_t timer_cap_gen_t;
struct timer_cap_gen_t
{
    uint32_t gpio_pin_config;      // GPIO_Pcx_TxCCPx, GPIO_Pcx_WTxCCPx

    uint32_t timer_val;            // Holds the last written or last read value of the timer
                                   // On initialization, set this to the number of microseconds
                                   // the timer should last for before overflowing so that the
                                   // load value and prescaler can be calculated. Use OVERFLOW_xxMS
                                   // defines.

    timer_cap_gen_t *linked_output;// If *this is an input, output the same pulse to the output

    uint8_t  gpio_port_ind;        // GPIO_Pc
    uint8_t  gpio_pin;             // GPIO_PIN_x

    uint8_t  timer_base_ind;       // TIMERx, WTIMERx
    uint8_t  timer_sel_ind;        // TIMERA, TIMERB

    uint8_t  capgen_mode;          // CAPGEN_MODE_?????
    int8_t   lifetimes_total;      // number of lifetimes of the sigtimeout timer before this
                                   // timer's signal is set to zero, negative to ignore
    int8_t   lifetimes_left;       // lifetimes left before this timer expires

};

/**
 * Initializes the default set of timers for the TIVA TM4C1232xx chip on the timing block board.
 * Must call this before using any of the other timer_default functions for this chip.
 */
void timer_default_init(void);

/**
 * Connects the given input to the given output using the default timer array. Used to pass
 * through PWMs.
 * \param input (timer_io_t) input timer label
 * \param output (timer_io_t) output timer label
 */
void timer_default_connect(uint8_t input, uint8_t output);

/**
 * Disconnects the given input timer from its output
 * \param input (timer_io_t) input timer label
 */
void timer_default_disconnect(uint8_t input);

/**
 * Disconnnects all default timers from their outputs
 */
void timer_default_disconnect_all(void);

/**
 * Measures or generates a pulse on the given io timer.
 * \param iotimer (timer_io_t) if an input, reads value of pwm pulse, if output, writes pulse
 * \param pulse_width_tics number of tics that make up the pulse width of the signal.
 *                         arg not used when reading an input-timer value
 * \return the measured or generated pulse width.
 */
uint32_t timer_default_pulse(uint8_t iotimer, uint32_t pulse_width_tics);

/**
 * Measures or generates a pulse on the given timer using RC standard PWM signals.
 * The pulse width value given maps from [0, UINT16_MAX] to a [05%, 10%] duty cycle
 * of the PWM signal.
 * \param iotimer (timer_io_t) if an input, reads value of pwm pulse, if output, writes pulse
 * \param pulse_width_RC number [0, UINT16_MAX] which mapes to [10%, 20%] duty cycle of the PWM
 *                              arg not used when reading an input-timer value
 * \return the measured or generated pulse width.
 */
uint16_t timer_default_pulse_RC(uint8_t iotimer, uint16_t pulse_width_RC);

/**
 * Generates the given pulse on all pwm output lines in the default timer array
 * \param pulse_width_tics number of tics that make up the pulse width of the signal.
 * \returns the generated pulse width.
 */
uint32_t timer_default_pulse_allpwm(uint32_t pulse_width_tics);

/**
 * Reads the pulse width of the given io timer
 * \param iotimer (timer_io_t) reads value of pwm pulse on this timer
 * \return the measured or generated pulse width.
 */
uint32_t timer_default_read_pulse(uint8_t iotimer);

uint64_t timer_default_get_total_load(uint8_t iotimer);

void timer_default_calc_ps_load_from_total(uint8_t iotimer, uint32_t *prescale,
                                            uint32_t *load, uint64_t total);

uint32_t timer_us_to_tics(uint32_t us);
uint32_t timer_tics_to_us(uint32_t tics);
uint32_t timer_pwm_to_ppm_RC_convention(uint32_t pwm_tics);


#endif /* TIMER_CAPTURE_GENERATE_H_ */
