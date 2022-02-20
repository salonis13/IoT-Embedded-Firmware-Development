/*
 * timers.h
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

#ifndef SRC_TIMERS_H_
#define SRC_TIMERS_H_

#include "src/timers.h"
#include "src/oscillators.h"
#include "app.h"

#include "em_letimer.h"
#include "main.h"

//Function to initialize LETIMER
void mytimer_init();

//function for polling delay
void timerWaitUs(uint32_t us_wait);

#endif /* SRC_TIMERS_H_ */
