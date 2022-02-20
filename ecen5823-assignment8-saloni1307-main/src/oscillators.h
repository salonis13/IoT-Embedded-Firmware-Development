/*
 * oscillators.h
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

#ifndef SRC_OSCILLATORS_H_
#define SRC_OSCILLATORS_H_

#include "em_cmu.h"
#include "main.h"

//Function to get the actual frequency of the oscillator after dividing prescaler
// No parameters
// Return value: actual frequency (integer)
int select_oscillator();

//Function to initialize, enable and set clock frequency
// No parameters, no return value
void oscillator_init();

#endif /* SRC_OSCILLATORS_H_ */
