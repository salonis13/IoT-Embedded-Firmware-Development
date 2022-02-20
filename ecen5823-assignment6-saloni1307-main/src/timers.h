/*
 * timers.h
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

#include "src/oscillators.h"
#include "app.h"

#include "em_letimer.h"

#define ACTUAL_CLK_FREQ select_oscillator()     //get actual clock frequency
#define VALUE_TO_LOAD_COMP0 (LETIMER_PERIOD_MS*ACTUAL_CLK_FREQ)/1000     //calculate value to load in COMP0

//Function to initialize LETIMER
void mytimer_init();

//function for interrupt based delay
void timerWaitUs_interrupt(uint32_t us_wait);

//function for polling delay
void timerWaitUs_polled(uint32_t us_wait);

#endif /* SRC_TIMERS_H_ */
