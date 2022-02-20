/*
 * scheduler.h
 *
 *  Created on: Sep 15, 2021
 *      Author: salon
 */

#ifndef SRC_SCHEDULER_H_
#define SRC_SCHEDULER_H_

#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "app.h"

#include "em_letimer.h"
#include "main.h"

//function to set a scheduler event
void schedulerSetEventUF();

//function to get scheduler event
uint32_t getNextEvent();


#endif /* SRC_SCHEDULER_H_ */
