/*
 * timers.c
 *
 * Functions to initialize and set LETIMER0
 *
 *  Created on: Sep 8, 2021
 *      Author: salon
 */

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "app.h"
#include "src/timers.h"

//resolution of LETIMER clock tick

#if (LOWEST_ENERGY_MODE < 3)
#define CLK_RES 61
#define MIN_WAIT 61

#else
#define CLK_RES 1000
#define MIN_WAIT 1000
#endif

#define MAX_WAIT 3000000  //maximum wait time possible

//structure to define parameters for LETIMER
const LETIMER_Init_TypeDef LETIMER_INIT_STRUCT = {
    false,              /* Disable timer when initialization completes. */
    true,              /* Allow counter to run during debug halt. */
    true,               /* load COMP0 into CNT on underflow. */
    false,              /* Do not load COMP1 into COMP0 when REP0 reaches 0. */
    0,                  /* Idle value 0 for output 0. */
    0,                  /* Idle value 0 for output 1. */
    letimerUFOANone,    /* No action on underflow on output 0. */
    letimerUFOANone,    /* No action on underflow on output 1. */
    letimerRepeatFree,  /* Count until stopped by SW. */
    0                   /* Use default value as top Value. */
};

void mytimer_init() {

  LETIMER_Init(LETIMER0, &LETIMER_INIT_STRUCT);         //initialize LETIMER0 using values defined in the structure
  LETIMER_CompareSet(LETIMER0, 0, VALUE_TO_LOAD_COMP0); //Set value of COMP0
  LETIMER_Enable(LETIMER0, true);                       //Enable LETIMER0

  //enable underflow interrupt of timer peripheral
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_UF);

}

//interrupt based delay of at least us_wait microseconds, using LETIMER0 tick counts as a reference
void timerWaitUs_interrupt(uint32_t us_wait) {
  uint16_t desired_tick, current_cnt, required_cnt;

  //check function argument range
  if((us_wait<(uint32_t)MIN_WAIT) || (us_wait>(uint32_t)MAX_WAIT)) {
      LOG_ERROR("TimerWait range\n\r");

      //clamp wait time value
      if(us_wait < (uint32_t)MIN_WAIT) {
          us_wait = MIN_WAIT;
      }

      else if(us_wait > (uint32_t)MAX_WAIT) {
          us_wait = MAX_WAIT;
      }
  }

  desired_tick = (us_wait/CLK_RES);           //calculate required timer ticks

  current_cnt = LETIMER_CounterGet(LETIMER0); //get current LETIMER counter value

  required_cnt = current_cnt-desired_tick;    //get required

  //handle roll-over case
  if(required_cnt > VALUE_TO_LOAD_COMP0) {

      //if required counter value is more than current counter value; wait till counter value is 0,
      //    recalculate remaining required ticks and poll for that much time
      required_cnt = VALUE_TO_LOAD_COMP0 - (0xFFFF - required_cnt);
  }

  LETIMER_CompareSet(LETIMER0, 1, required_cnt); //Set value of COMP1

  //enable COMP1 interrupt of timer peripheral
  LETIMER_IntEnable(LETIMER0, LETIMER_IEN_COMP1);

  LETIMER0->IEN |= LETIMER_IEN_COMP1;

}


//blocks (polls) at least us_wait microseconds, using LETIMER0 tick counts as a reference
void timerWaitUs_polled(uint32_t us_wait) {
  uint16_t desired_tick, current_cnt, required_cnt;

  //check function argument range
  if((us_wait<(uint32_t)MIN_WAIT) | (us_wait>(uint32_t)MAX_WAIT)) {
      LOG_ERROR("TimerWait range\n\r");

      //clamp wait time value
      if(us_wait < (uint16_t)MIN_WAIT) {
          us_wait = MIN_WAIT;
      }

      else if(us_wait > (uint16_t)MAX_WAIT) {
          us_wait = MAX_WAIT;
      }
  }

  desired_tick = (us_wait/CLK_RES);           //calculate required timer ticks

  current_cnt = LETIMER_CounterGet(LETIMER0); //get current LETIMER counter value

  required_cnt = current_cnt-desired_tick;    //get required number of timer ticks

  //wait time value wrap around case
  /* if(required_cnt >= (uint16_t)VALUE_TO_LOAD_COMP0) {
      required_cnt = 0xFFFF - required_cnt;

      required_cnt = VALUE_TO_LOAD_COMP0 - required_cnt;
  }*/

  //poll until required time period passes
  if(current_cnt >= desired_tick) {
      while((LETIMER_CounterGet(LETIMER0)) != (required_cnt));
  }

  //handle roll-over case
  else {
      //if required counter value is more than current counter value; wait till counter value is 0,
      //    recalculate remaining required ticks and poll for that much time
      while((LETIMER_CounterGet(LETIMER0)) != (uint32_t)(VALUE_TO_LOAD_COMP0-(desired_tick-current_cnt)));
  }

}


