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

  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // determine pending interrupts in peripheral
  uint32_t reason = LETIMER_IntGetEnabled(LETIMER0);
  // clear pending interrupts in peripheral
  LETIMER_IntClear(LETIMER0, reason);

  CORE_EXIT_CRITICAL();

  //logic to turn on led after 2075 ms and turn off after 2250 ms
  if(reason==2) {
    gpioLed0SetOff();
    LOG_INFO("off\n\r");
  }

  else if(reason==4) {
    gpioLed0SetOn();
    LOG_INFO("on\n\r");
  }

}
