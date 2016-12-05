#ifndef TMP102_H_INCLUDED
#define TMP102_H_INCLUDED

#include "kubos-hal/i2c.h"

#define I2C_DEV K_I2C2
#define I2C_SLAVE_ADDR 0x48

void tmp102_setup();
void tmp102_read_temperature(int *);

#endif