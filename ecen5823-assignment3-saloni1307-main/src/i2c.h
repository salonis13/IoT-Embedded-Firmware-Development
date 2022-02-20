/*
 * i2c.h
 *
 *  Created on: Sep 16, 2021
 *      Author: salon
 */

#ifndef SRC_I2C_H_
#define SRC_I2C_H_

#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "app.h"

#include "em_letimer.h"
#include "em_gpio.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "main.h"

void i2c_init();

float convertTemp();

void write_cmd();

void read_cmd();

void read_temp_from_si7021();

#endif /* SRC_I2C_H_ */
