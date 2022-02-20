/*
 * pulse_oximeter.c
 *
 *  Created on: Nov 19, 2021
 *      Author: mich1576
 */

/*
 SDA -> SDA
 SCL -> SCL
 RESET ->
 MFIO ->

 Error codes
 1 = Unavailable Command
 2 = Unavailable Function
 3 = Data Format Error
 4 = Input Value Error
 5 = Try Again
 255 = Error Unknown
*/

#include "pulse_oximeter.h"
// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

uint8_t set_output_mode[3] = {0x10, 0x00, 0x02};  //set the output mode
uint8_t setFifoThreshold[3] = {0x10, 0x01, 0x01}; //set threshold fifo, One sample before interrupt is fired.
uint8_t agcAlgoControl[3] = {0x52, 0x00, 0x01}; // One sample before interrupt is fired.
uint8_t max30101Control[3] = {0x44, 0x03, 0x01}; //
uint8_t maximFastAlgoControl[3] = {0x52, 0x02, 0x01}; //
uint8_t readAlgoSamples[3] = {0x51, 0x00, 0x03};
uint8_t read_sensor_hub_status[2] = {0x00, 0x00};
uint8_t numSamplesOutFifo[2] = {0x12, 0x00};
uint8_t read_fill_array[2] = {0x12, 0x01};
uint8_t disable_AFE[3] = {0x44, 0x03, 00};
uint8_t disable_algo[3] = {0x52, 0x02, 0x00};


void pulse_oximeter_init_pins(){

  GPIO_DriveStrengthSet(MAX30101_port, gpioDriveStrengthStrongAlternateStrong);
  GPIO_PinModeSet(MAX30101_port, RESET_pin, gpioModePushPull, true);
  GPIO_PinModeSet(MAX30101_port, MFIO_pin, gpioModePushPull, false);
}

void turn_off_reset(){
  GPIO_PinOutClear(MAX30101_port, RESET_pin);
}

void turn_on_reset(){
  GPIO_PinOutSet(MAX30101_port, RESET_pin);
}

void turn_off_mfio(){
  GPIO_PinOutClear(MAX30101_port, MFIO_pin);
}

void turn_on_mfio(){
  GPIO_PinOutSet(MAX30101_port, MFIO_pin);
}

void set_MFIO_interrupt(){
  GPIO_PinModeSet(MAX30101_port, MFIO_pin, gpioModeInputPull, true);
  GPIO_ExtIntConfig (MAX30101_port, MFIO_pin, MFIO_pin, false, true, true);

}

void set_output_mode_func(){

  //set_output_mode
  I2C_pulse_write_polled(set_output_mode, 3);
}

void setFifoThreshold_func(){

  //set fifo threshold
  I2C_pulse_write_polled(setFifoThreshold, 3);
}

void agcAlgoControl_func(){

  //agcAlgoControl
  I2C_pulse_write_polled(agcAlgoControl, 3);

}

void max30101Control_func(){

  //max30101Control
  I2C_pulse_write_polled(max30101Control, 3);

}

void maximFastAlgoControl_func(){

  // Family Byte: ENABLE_ALGORITHM (0x52), Index Byte:
  // ENABLE_WHRM_ALGO (0x02)
  // This function enables (one) or disables (zero) the wrist heart rate monitor
  // algorithm.
    I2C_pulse_write_polled(maximFastAlgoControl, 3);
}


void readAlgoSamples_func(){

  // Family Byte: READ_ALGORITHM_CONFIG (0x51), Index Byte:
  // READ_AGC_NUM_SAMPLES (0x00), Write Byte: READ_AGC_NUM_SAMPLES_ID (0x03)
  // This function changes the number of samples that are averaged.
  // It takes a paramater of zero to 255.
    I2C_pulse_write_polled(readAlgoSamples, 3);

}

void read_sensor_hub_status_func(){

  I2C_pulse_write_polled(read_sensor_hub_status, 2);
}

void numSamplesOutFifo_func(){

  I2C_pulse_write_polled(numSamplesOutFifo,2);

}

void read_fill_array_func(){

  I2C_pulse_write_polled(read_fill_array, 2);
}

void disable_AFE_func(){
  I2C_pulse_write_polled(disable_AFE,3);
}

void disable_algo_func(){
  I2C_pulse_write_polled(disable_algo,3);
}
