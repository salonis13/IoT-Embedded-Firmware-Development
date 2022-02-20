/*
 * irq.h
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

#ifndef SRC_IRQ_H_
#define SRC_IRQ_H_

#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "src/scheduler.h"
#include "app.h"

#include "em_letimer.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "main.h"

uint32_t letimerMilliseconds();

// LETIMER0 interrupt handler
void LETIMER0_IRQHandler(void);

//I2C0 interrupt handler
void I2C0_IRQHandler();


#endif /* SRC_IRQ_H_ */
