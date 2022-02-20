/*
 * pulse_oximeter.h
 *
 *  Created on: Nov 19, 2021
 *      Author: mich1576
 */

#ifndef SRC_PULSE_OXIMETER_H_
#define SRC_PULSE_OXIMETER_H_

#include "i2c.h"
#include "gpio.h"
#include <stdbool.h>

#define MAX30101_port gpioPortA
#define MFIO_pin 2

#define RESET_pin 3

void turn_off_reset();

void turn_on_reset();

void turn_off_mfio();

void turn_on_mfio();

void set_MFIO_interrupt();

void pulse_oximeter_init_pins();

void set_output_mode_func();

void setFifoThreshold_func();

void agcAlgoControl_func();

void max30101Control_func();

void maximFastAlgoControl_func();

void readAlgoSamples_func();

void read_sensor_hub_status_func();

void numSamplesOutFifo_func();

void read_fill_array_func();

void disable_AFE_func();

void disable_algo_func();

#endif /* SRC_PULSE_OXIMETER_H_ */
