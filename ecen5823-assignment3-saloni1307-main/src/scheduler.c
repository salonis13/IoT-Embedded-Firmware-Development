/*
 * scheduler.c
 *
 *  Created on: Sep 15, 2021
 *      Author: salon
 */

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

#include "app.h"
#include "src/scheduler.h"

uint32_t MyEvent;

//enum to define scheduler events
enum {
  evtNoEvent = 0,
  evtLETIMER0_UF = 1,
};

// scheduler routine to set a scheduler event
void schedulerSetEventUF() {

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // set the event in your data structure, this is a read-modify-write
  MyEvent = evtLETIMER0_UF;
  // exit critical section
  CORE_EXIT_CRITICAL();
} // schedulerSetEventXXX()


// scheduler routine to return 1 event to main()code and clear that event
uint32_t getNextEvent() {
  uint32_t theEvent;
  //determine 1 event to return to main() code, apply priorities etc.
  theEvent = MyEvent;

  // enter critical section
  CORE_DECLARE_IRQ_STATE;
  CORE_ENTER_CRITICAL();

  // clear the event in your data structure, this is a read-modify-write
  MyEvent = evtNoEvent;
  // exit critical section
  CORE_EXIT_CRITICAL();

  return (theEvent);
} // getNextEvent()
