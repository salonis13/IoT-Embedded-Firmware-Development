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
#include "app.h"

#include "em_letimer.h"
#include "main.h"

// LETIMER0 interrupt handler
void LETIMER0_IRQHandler(void);


#endif /* SRC_IRQ_H_ */
