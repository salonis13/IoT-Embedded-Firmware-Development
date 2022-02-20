/*
  gpio.c

   Created on: Dec 12, 2018
       Author: Dan Walkes
   Updated by Dave Sluiter Dec 31, 2020. Minor edits with #defines.

   March 17
   Dave Sluiter: Use this file to define functions that set up or control GPIOs.

 */




#include "src/gpio.h"
#include "app.h"
#include "main.h"





// Set GPIO drive strengths and modes of operation
void gpioInit()
{

  // Student Edit:

  //GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED0_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED0_port, LED0_pin, gpioModePushPull, false);

  //GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_DriveStrengthSet(LED1_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(LED1_port, LED1_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(sensor_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(sensor_port, sensor_pin, gpioModePushPull, false);

  GPIO_DriveStrengthSet(lcd_port, gpioDriveStrengthWeakAlternateWeak);
  GPIO_PinModeSet(lcd_port, lcd_pin, gpioModePushPull, false);

  GPIO_PinModeSet(PB0_port, PB0_pin, gpioModeInputPullFilter, true);
  GPIO_ExtIntConfig(PB0_port, PB0_pin, PB0_pin, true, true, true);

  GPIO_PinModeSet(PB1_port, PB1_pin, gpioModeInputPullFilter, true);
  GPIO_ExtIntConfig(PB1_port, PB1_pin, PB1_pin, true, true, true);

} // gpioInit()


void gpioLed0SetOn()
{
  GPIO_PinOutSet(LED0_port,LED0_pin);
}


void gpioLed0SetOff()
{
  GPIO_PinOutClear(LED0_port,LED0_pin);
}


void gpioLed1SetOn()
{
  GPIO_PinOutSet(LED1_port,LED1_pin);
}


void gpioLed1SetOff()
{
  GPIO_PinOutClear(LED1_port,LED1_pin);
}

void enable_sensor() {
  GPIO_PinOutSet(sensor_port, sensor_pin);
}

void disable_sensor() {
  GPIO_PinOutClear(sensor_port, sensor_pin);
}

void gpioSetDisplayExtcomin(bool value) {

  if(value == true) {
      GPIO_PinOutSet(lcd_port, lcd_pin);
  }
  else {
      GPIO_PinOutClear(lcd_port, lcd_pin);
  }
}




