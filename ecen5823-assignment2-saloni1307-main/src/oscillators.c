/*
 * oscillators.c
 *
 * Function to initialize and set oscillator type and frequencies
 *  for different energy modes
 *
 *  Created on: Sep 8, 2021
 *      Author: saloni
 */

#include "src/oscillators.h"
#include "app.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#define LFXO_FREQ   32768   //frequency of low frequency crystal oscillator
#define LFXO_PRESCALER 4    //prescaler for low frequency oscillator
#define ULFRCO_FREQ 1000    //frequency for ultra low frequency RC oscillator
#define ULFRCO_PRESCALER 1  //prescaler for ultra low frequency RC oscillator

int select_oscillator() {

  //get actual clock frequency for EM3 mode
  if(LOWEST_ENERGY_MODE == 3) {
      return (ULFRCO_FREQ/ULFRCO_PRESCALER);
  }

  //get actual frequency for other energy modes
  else {
      return (LFXO_FREQ/LFXO_PRESCALER);
  }

}

void oscillator_init() {

  int lfa_clk=0, letimer_clk=0;   //variables to check clock frequencies

  //clock set for EM3 energy mode
  if(LOWEST_ENERGY_MODE == 3) {
      CMU_OscillatorEnable(cmuOsc_ULFRCO, true, true);    //enable ULFRCO oscillator
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_ULFRCO); //select ULFRCO oscillator for low frequency peripheral branch
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_1);    //set prescaler for LETIMER0 peripheral
      CMU_ClockEnable(cmuClock_LETIMER0, true);           //enable clock for LETIMER0 peripheral
      lfa_clk = CMU_ClockFreqGet(cmuClock_LFA);           //get clock frequency for low frequency branch
      letimer_clk = CMU_ClockFreqGet(cmuClock_LETIMER0);  //get clock frequency for LETIMER
      LOG_INFO("lfa_clk=%d,  letimer_clk=%d in em3\n\r", (int32_t)lfa_clk, (int32_t)letimer_clk);  //log clock frequencies
  }

  //clock set for EM0, EM1 and EM2 energy modes
  else {
      CMU_OscillatorEnable(cmuOsc_LFXO, true, true);      //enable LFXO oscillator
      CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);   //select LFXO oscillator for low frequency peripheral branch
      CMU_ClockDivSet(cmuClock_LETIMER0, cmuClkDiv_4);    //set prescaler for LETIMER0 peripheral
      CMU_ClockEnable(cmuClock_LETIMER0, true);           //enable clock for LETIMER0 peripheral
      lfa_clk = CMU_ClockFreqGet(cmuClock_LFA);           //get clock frequency for low frequency branch
      letimer_clk = CMU_ClockFreqGet(cmuClock_LETIMER0);  //get clock frequency for LETIMER
      LOG_INFO("lfa_clk=%d,  letimer_clk=%d\n\r", (int32_t)lfa_clk, (int32_t)letimer_clk);   //log clock frequencies
  }

}


