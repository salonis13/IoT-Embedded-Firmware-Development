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

// BLE Data Structure, save all of our private BT data in here.
// Modern C (circa 2021 does it this way)
// typedef ble_data_struct_t is referred to as an anonymous struct definition

typedef struct {

  // values that are common to servers and clients
  bd_addr myAddress;
  uint8_t myAddressType;

  // values unique for server
  // The advertising set handle allocated from Bluetooth stack.
  uint8_t advertisingSetHandle;\
  //connection handle
  uint8_t connection_handle;
  //flag to check if bluetooth is connected
  bool    connected;
  //flag to check if indication is on
  bool    indication;
  //flag to check if indication is in flight
  bool    indication_inFlight;
  //rollover count variable
  uint32_t rollover_cnt;

  // values unique for client}
} ble_data_struct_t;

ble_data_struct_t* getBleDataPtr(void);

void ble_SendTemp();

void handle_ble_event(sl_bt_msg_t *evt);

#endif /* SRC_BLE_H_ */
