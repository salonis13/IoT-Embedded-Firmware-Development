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

uint32_t letimerMilliseconds() {
  uint32_t time_ms;
  ble_data_struct_t *bleData = getBleDataPtr();
  time_ms = ((bleData->rollover_cnt)*3000);
  return time_ms;
}

void LETIMER0_IRQHandler(void) {

  ble_data_struct_t *bleData = getBleDataPtr();

  // determine pending interrupts in peripheral
  uint32_t reason = LETIMER_IntGetEnabled(LETIMER0);
  // clear pending interrupts in peripheral
  LETIMER_IntClear(LETIMER0, reason);

  //check for COMP1 interrupt
  if(reason & LETIMER_IF_COMP1) {

      //disable COMP1 interrupt of timer peripheral
      LETIMER_IntDisable(LETIMER0, LETIMER_IEN_COMP1);

      //set scheduler event
      schedulerSetEventCOMP1();
  }

  //check for UF interrupt
  if(reason & LETIMER_IF_UF) {

      //set scheduler event
      schedulerSetEventUF();

      bleData->rollover_cnt+=1;

  }

}

void I2C0_IRQHandler(void) {

  I2C_TransferReturn_TypeDef transferStatus;

  //get I2C transfer status
  transferStatus = I2C_Transfer(I2C0);

  //check if I2C transfer is done
  if(transferStatus == i2cTransferDone) {

      //disable I2C transfer
      NVIC_DisableIRQ(I2C0_IRQn);

      //set scheduler event
      schedulerSetEventTransferDone();
  }

  if(transferStatus < 0) {

      LOG_ERROR("I2C_TStatus %d : failed\n\r", (uint32_t)transferStatus);

  }
}

void GPIO_EVEN_IRQHandler(void) {

  ble_data_struct_t *bleData = getBleDataPtr();

  // determine pending interrupts in peripheral
  uint32_t reason = GPIO_IntGet();

  GPIO_IntClear(reason);

  //get the push button status
  uint8_t button_status = GPIO_PinInGet(PB0_port, PB0_pin);

  //check if the interrupt triggered was from PB0
  if(reason == 64) {

      if(!button_status) {
          bleData->button_pressed = true;
          schedulerSetEventButtonPressed();
      }

      else {
          bleData->button_pressed = false;
          schedulerSetEventButtonReleased();
      }
  }
}

#if !DEVICE_IS_BLE_SERVER

void GPIO_ODD_IRQHandler(void) {

  ble_data_struct_t *bleData = getBleDataPtr();

  // determine pending interrupts in peripheral
  uint32_t reason = GPIO_IntGet();

  GPIO_IntClear(reason);

  //get the push button status
  uint8_t button_status = GPIO_PinInGet(PB1_port, PB1_pin);

  //check if the interrupt triggered was from PB1
  if(reason == 128) {

      if(!button_status) {
          bleData->pb1_button_pressed = true;
          schedulerSetEventButtonPressed();
      }

      else {
          bleData->pb1_button_pressed = false;
          schedulerSetEventButtonReleased();
      }
  }
}

#endif
