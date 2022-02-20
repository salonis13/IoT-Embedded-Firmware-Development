/*
 * ble.h
 *
 *  Created on: Sep 30, 2021
 *      Author: salon
 */

#ifndef SRC_BLE_H_
#define SRC_BLE_H_

#include "src/timers.h"
#include "src/oscillators.h"
#include "src/gpio.h"
#include "src/i2c.h"
#include "app.h"

#include "em_letimer.h"
#include "em_gpio.h"
#include "sl_i2cspm.h"
#include "em_i2c.h"
#include "main.h"

#define UINT8_TO_BITSTREAM(p, n)      { *(p)++ = (uint8_t)(n); }
#define UINT32_TO_BITSTREAM(p, n)     { *(p)++ = (uint8_t)(n); *(p)++ = (uint8_t)((n) >> 8); \
    *(p)++ = (uint8_t)((n) >> 16); *(p)++ = (uint8_t)((n) >> 24); }
#define UINT32_TO_FLOAT(m, e)         (((uint32_t)(m) & 0x00FFFFFFU) | (uint32_t)((int32_t)(e) << 24))

#define MAX_PTR 16
#define READ 0
#define WRITE 1

// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition

//private structure for indication data
struct buffer_entry
{
  uint16_t charHandle;
  size_t bufferLength;
  uint8_t buffer[5];
};

typedef struct {

  // values that are common to servers and clients

  //server address
  bd_addr myAddress;
  uint8_t myAddressType;

  // values unique for server

  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertisingSetHandle;
  //connection handle
  uint8_t connection_handle;
  //flag to check if bluetooth is connected
  bool    connected;
  //flag to check if HTM indication is on
  bool    indication;
  //flag to check if push button indication is on
  bool button_indication;

  bool gesture_indication;

  bool oximeter_indication;
  //flag to check if indication is in flight
  bool    indication_inFlight;
  //rollover count variable
  uint32_t rollover_cnt;
  //flag to check if push button is pressed
  bool button_pressed;
  bool pb1_button_pressed;
  //flag to check if server and client are bonded
  bool bonded;
  //variable to save bonding passkey
  uint32_t passkey;

  // values unique for client
  //temperature service handle
  uint32_t service_handle;
  //temperature characteristic handle
  uint16_t char_handle;
  //indication characteristic value from server
  uint8_t * char_value;
  //button service handle
  uint32_t button_service_handle;
  //button characteristic handle
  uint16_t button_char_handle;

  //gesture service handle
  uint32_t gesture_service_handle;
  //gesture characteristic handle
  uint16_t gesture_char_handle;
  //gesture value
  uint8_t gesture_value;
  //gesture on
  bool gesture_on;

  //button service handle
  uint32_t oximeter_service_handle;
  //button characteristic handle
  uint16_t oximeter_char_handle;
  //turn off oximeter sensor
  bool oximeter_off;


} ble_data_struct_t;

ble_data_struct_t* getBleDataPtr(void);

#if DEVICE_IS_BLE_SERVER

void ble_SendTemp();

void ble_SendButtonState(uint8_t value);

void ble_SendGestureState(uint8_t value);

void ble_SendOximeterState(uint8_t* pulse_data);

#endif

void handle_ble_event(sl_bt_msg_t *evt);

#endif /* SRC_BLE_H_ */
