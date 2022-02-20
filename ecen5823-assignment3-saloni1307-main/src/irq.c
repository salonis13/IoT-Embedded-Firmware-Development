/*
 * irq.c
 *
 * Function to
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "app.h"
#include "src/irq.h"

void LETIMER0_IRQHandler(void) {

  // determine pending interrupts in peripheral
  uint32_t reason = LETIMER_IntGetEnabled(LETIMER0);
  // clear pending interrupts in peripheral
  LETIMER_IntClear(LETIMER0, reason);

  //set scheduler event
  schedulerSetEventUF();
}
