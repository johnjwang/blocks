/*
 * i2c_comms.c
 *
 *  Created on: Oct 13, 2014
 *      Author: Jonathan
 */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_memmap.h"

#include "driverlib/gpio.h"
#include "driverlib/i2c.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"

#include "i2c_comms.h"

#include "io/comms.h"

static void i2c_comms_int_handler(void);

static comms_t *i2c_comms = NULL;

void i2c_comms_init(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);

    GPIOPinConfigure(GPIO_PE4_I2C2SCL);
    GPIOPinConfigure(GPIO_PE5_I2C2SDA);

    GPIOPinTypeI2C(GPIO_PORTE_BASE, GPIO_PIN_4 | GPIO_PIN_5);
    GPIOPinTypeI2CSCL(GPIO_PORTE_BASE, GPIO_PIN_4);

    i2c_comms = comms_create(100, 100);
    comms_add_publisher(i2c_comms, i2c_comms_write);

    // Use the system clock for the I2C module.  Set the I2C data transfer rate to 400kbps.
    I2CMasterInitExpClk(I2C2_BASE, SysCtlClockGet(), true);

    I2CMasterEnable(I2C2_BASE);
    I2CSlaveEnable(I2C2_BASE);

    I2CMasterIntEnable(I2C2_BASE);
    I2CMasterIntEnableEx(I2C2_BASE, I2C_MASTER_INT_ARB_LOST | I2C_MASTER_INT_TIMEOUT | I2C_MASTER_INT_STOP | I2C_MASTER_INT_NACK);

    I2CSlaveIntEnable(I2C2_BASE);
    I2CMasterIntEnableEx(I2C2_BASE, I2C_SLAVE_INT_DATA);

    I2CIntRegister(I2C2_BASE, i2c_comms_int_handler);
}

bool i2c_comms_write_byte(uint8_t byte);
bool i2c_comms_write(uint8_t *msg, uint16_t dataLen);
void i2c_comms_publish(comms_channel_t channel, uint8_t *msg, uint16_t msg_len);
void i2c_comms_subscribe(comms_channel_t channel, subscriber_t subscriber, void *usr);
void i2c_comms_demo(void);

static void i2c_comms_int_handler(void)
{
    uint32_t master_mode = I2CMasterIntStatusEx(I2C2_BASE, true);
    I2CMasterIntClearEx(I2C2_BASE, master_mode);
    uint32_t slave_mode  = I2CSlaveIntStatusEx(I2C2_BASE, true);
    I2CSlaveIntClearEx(I2C2_BASE, slave_mode);
}
