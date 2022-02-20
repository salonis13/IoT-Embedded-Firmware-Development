/*
   gpio.h
  
    Created on: Dec 12, 2018
        Author: Dan Walkes

    Updated by Dave Sluiter Sept 7, 2020. moved #defines from .c to .h file.
    Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

 */

#ifndef SRC_GPIO_H_
#define SRC_GPIO_H_
#include <stdbool.h>
#include "em_gpio.h"
#include <string.h>


// Student Edit: Define these, 0's are placeholder values.
// See the radio board user guide at https://www.silabs.com/documents/login/user-guides/ug279-brd4104a-user-guide.pdf
// and GPIO documentation at https://siliconlabs.github.io/Gecko_SDK_Doc/efm32g/html/group__GPIO.html
// to determine the correct values for these.

//LED0 is on PF 4 pin and LED1 is on PF5 pin
#define	LED0_port  5 // change to correct ports and pins
#define LED0_pin   4
#define LED1_port  5
#define LED1_pin   5







// Function prototypes

//function to initialize GPIO
void gpioInit();

//function to set LED0 on
void gpioLed0SetOn();

//function to set LED0 off
void gpioLed0SetOff();

//function to set LED1 on
void gpioLed1SetOn();

//function to set LED1 off
void gpioLed1SetOff();





#endif /* SRC_GPIO_H_ */
