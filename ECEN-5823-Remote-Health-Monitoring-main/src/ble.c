/*
 * ble.c
 *
 *  Created on: Sep 30, 2021
 *      Author: salon
 */

#include "src/ble.h"
#include "app.h"

// Include logging for this file
#define INCLUDE_LOG_DEBUG 1
#include "src/log.h"

//turn this on to log the parameters set by server
#define LOG_PARAMETER_VALUES 0
#define FLAGS 0x0F

#define SCAN_PASSIVE 0

//enum for interrupt based events
enum {
  evt_NoEvent=0,
  evt_TimerUF,
  evt_COMP1,
  evt_TransferDone,
  evt_ButtonPressed,
  evt_ButtonReleased,
};

// BLE private data
ble_data_struct_t ble_data;

//array for hard-coded server address
uint8_t server_addr[6] = SERVER_BT_ADDRESS;

#if !DEVICE_IS_BLE_SERVER
// Health Thermometer service UUID defined by Bluetooth SIG
static const uint8_t thermo_service[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
static const uint8_t thermo_char[2] = { 0x1c, 0x2a };

// button state service UUID defined by Bluetooth SIG
// 00000001-38c8-433e-87ec-652a2d136289
static const uint8_t button_service[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x01, 0x00, 0x00, 0x00 };
// button state characteristic UUID defined by Bluetooth SIG
// 00000002-38c8-433e-87ec-652a2d136289
static const uint8_t button_char[16] = { 0x89, 0x62, 0x13, 0x2d, 0x2a, 0x65, 0xec, 0x87, 0x3e, 0x43, 0xc8, 0x38, 0x02, 0x00, 0x00, 0x00 };

//gesture sensor service uuid
//d38b2e9b-966c-4426-bda0-9c017e23bb35
static const uint8_t gesture_service[16] = { 0x35, 0xbb, 0x23, 0x7e, 0x01, 0x9c, 0xa0, 0xbd, 0x26, 0x44, 0x6c, 0x96, 0x9b, 0x2e, 0x8b, 0xd3 };
//gesture sensor characteristic uuid
//e38b2e9b-966c-4426-bda0-9c017e23bb35
static const uint8_t gesture_char[16] = { 0x35, 0xbb, 0x23, 0x7e, 0x01, 0x9c, 0xa0, 0xbd, 0x26, 0x44, 0x6c, 0x96, 0x9b, 0x2e, 0x8b, 0xe3 };

//pulse oximeter service uuid
//ee63a26e-8809-4038-8c7c-341ed4042818
static const uint8_t oximeter_service[16] = { 0x18, 0x28, 0x04, 0xd4, 0x1e, 0x34, 0x7c, 0x8c, 0x38, 0x40, 0x09, 0x88, 0x6e, 0xa2, 0x63, 0xee};
//pulse oximeter characteristic uuid
//fe63a26e-8809-4038-8c7c-341ed4042818
static const uint8_t oximeter_char[16] = { 0x18, 0x28, 0x04, 0xd4, 0x1e, 0x34, 0x7c, 0x8c, 0x38, 0x40, 0x09, 0x88, 0x6e, 0xa2, 0x63, 0xfe};

#endif

sl_status_t sc=0;
int qc=0;

uint32_t adv_int=0x190;           //250 ms advertisement interval
uint16_t conn_int = 0x3c;         //75 ms connection interval
uint16_t slave_latency = 0x03;    //3 slave latency - slave can skip upto 3 connection events
uint16_t spvsn_timeout = 0x50;   //800ms supervision timeout
uint16_t scan_int = 0x50;         //scanning interval of 50 ms
uint16_t scan_window = 0x28;      //scanning window of 25 ms

ble_data_struct_t * getBleDataPtr() {

  return (&ble_data);
}

#if DEVICE_IS_BLE_SERVER

//function to send temperature value indication to client
void ble_SendTemp() {

  uint8_t htm_temperature_buffer[5];
  uint8_t *p = htm_temperature_buffer;
  uint32_t htm_temperature_flt;
  uint8_t flags = 0x00;
  ble_data_struct_t *bleData = getBleDataPtr();

  //struct buffer_entry indication_data;

  //check if bluetooth is connected
  if((bleData->connected == true) && (bleData->bonded == true)){

      LOG_INFO("Sending temp indication\n\r");
      //get temperature value from sensor in celcius
      float temperature_in_c = convertTemp();

      UINT8_TO_BITSTREAM(p, flags);

      htm_temperature_flt = UINT32_TO_FLOAT(temperature_in_c*1000, -3);

      UINT32_TO_BITSTREAM(p, htm_temperature_flt);

      //write temperature value in gatt database
      sl_status_t sc = sl_bt_gatt_server_write_attribute_value(gattdb_temperature_measurement,
                                                               0,
                                                               5,
                                                               &htm_temperature_buffer[0]);

      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

          //check if any indication is inFlight
          if(bleData->indication_inFlight) {
              LOG_ERROR("Indication in flight\n\r");
          }

          //send indication of temperature measurement if no indication is inFlight
          else {
              sc = sl_bt_gatt_server_send_indication(bleData->connection_handle,
                                                     gattdb_temperature_measurement,
                                                     5,
                                                     &htm_temperature_buffer[0]);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
              else {

                  //indication is sent i.e. indication is in flight
                  bleData->indication_inFlight = true;
                  LOG_INFO("Sent HTM indication, temp=%f\n\r", temperature_in_c);
                  displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%f", temperature_in_c);
              }
          }
  }

}

//function to send button state indication
void ble_SendButtonState(uint8_t value) {

  ble_data_struct_t *bleData = getBleDataPtr();

  //struct buffer_entry indication_data;

  uint8_t button_value_buffer[2];

  button_value_buffer[0] = value;
  button_value_buffer[1] = 0;

  //check if Bluetooth is connected
  if(bleData->connected == true){

      //write button state in gatt database
      sl_status_t sc = sl_bt_gatt_server_write_attribute_value(gattdb_button_state,
                                                               0,
                                                               1,
                                                               &button_value_buffer[0]);

      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      //check if indication is on and server and client are bonded
      if (bleData->bonded == true) {

          //check if any indication is inFlight
          if(bleData->indication_inFlight) {

          }

          //send indication of temperature measurement if no indication is inFlight
          else {
              sc = sl_bt_gatt_server_send_indication(bleData->connection_handle,
                                                     gattdb_button_state,
                                                     2,
                                                     &button_value_buffer[0]);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }

              else {

                  //indication is sent i.e. indication is in flight
                  bleData->indication_inFlight = true;
                  //LOG_INFO("Sent Button indication, value=%d\n\r", value);
              }
          }
      }
  }

}

//function to send gesture indication
void ble_SendGestureState(uint8_t value) {

  ble_data_struct_t *bleData = getBleDataPtr();

  //struct buffer_entry indication_data;

  uint8_t gesture_value_buffer[2];

  gesture_value_buffer[0] = value;
  gesture_value_buffer[1] = 0;

  //check if Bluetooth is connected
  if(bleData->connected == true){

      //write button state in gatt database
      sl_status_t sc = sl_bt_gatt_server_write_attribute_value(gattdb_gesture_state,
                                                               0,
                                                               1,
                                                               &gesture_value_buffer[0]);

      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      //check if indication is on and server and client are bonded
      if (bleData->bonded) {

          //check if any indication is inFlight
          if(bleData->indication_inFlight) {
              LOG_INFO("Cannot send gesture reading, indication inflight\n\r");
          }

          //send indication of gesture measurement if no indication is inFlight
          else {
              sc = sl_bt_gatt_server_send_indication(bleData->connection_handle,
                                                     gattdb_gesture_state,
                                                     2,
                                                     &gesture_value_buffer[0]);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }

              else {

                  //indication is sent i.e. indication is in flight
                  bleData->indication_inFlight = true;
                  LOG_INFO("Sent gesture indication, value=%d\n\r", value);
              }
          }
      }
  }

}

//function to send oximeter and heart rate indication
void ble_SendOximeterState(uint8_t* pulse_data) {

  ble_data_struct_t *bleData = getBleDataPtr();

  //check if Bluetooth is connected
  if(bleData->connected == true){

      LOG_INFO("Does it send indication?\n\r");
      //write button state in gatt database
      sl_status_t sc = sl_bt_gatt_server_write_attribute_value(gattdb_oximeter_state,
                                                               0,
                                                               2,
                                                               &pulse_data[0]);

      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_gatt_server_write_attribute_value() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      //check if indication is on and server and client are bonded
      if (bleData->bonded) {

          //check if any indication is inFlight
          if(bleData->indication_inFlight) {
              LOG_INFO("Cannot send oximeter reading, indication inflight\n\r");
          }

          //send indication of oximeter measurement if no indication is inFlight
          else {
              sc = sl_bt_gatt_server_send_indication(bleData->connection_handle,
                                                     gattdb_oximeter_state,
                                                     2,
                                                     &pulse_data[0]);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }

              else {

                  //indication is sent i.e. indication is in flight
                  bleData->indication_inFlight = true;
                  LOG_INFO("Sent oximeter indication\n\r");
              }
          }
      }
  }

}

#else

int32_t temperature_in_c;
// convert IEEE-11073 32-bit float to integer
static  int32_t  FLOAT_TO_INT32(const  uint8_t  *value_start_little_endian)
{
  uint8_t signByte = 0;
  int32_t mantissa;
  // input data format is:
  // [0]       = flags byte
  // [3][2][1] = mantissa (2's complement)
  // [4]       = exponent (2's complement)
  // BT value_start_little_endian[0] has the flags byte
  int8_t  exponent = (int8_t)value_start_little_endian[4];
  // sign extend the mantissa value if the mantissa is negative
  if  (value_start_little_endian[3] & 0x80) {
      //  msb  of [3] is the sign of the mantissa
      signByte = 0xFF;

  }
  mantissa = (int32_t) (value_start_little_endian[1]  << 0)  |
      (value_start_little_endian[2]  << 8)  |
      (value_start_little_endian[3]  << 16) |
      (signByte                      << 24) ;
  // value = 10^exponent * mantissa,  pow() returns a double type
  return  (int32_t) (pow(10, exponent) * mantissa); // FLOAT_TO_INT32

}

#endif

//function to handle BLE events
void handle_ble_event(sl_bt_msg_t *evt) {

  ble_data_struct_t *bleData = getBleDataPtr();

  //check ble event
  switch(SL_BT_MSG_ID(evt->header)) {

    //for both server and client
    //system boot event
    //Indicates that the device has started and the radio is ready
    case sl_bt_evt_system_boot_id:

      displayInit();

      //LOG_INFO("Boot event\n\r");

      /*Read the Bluetooth identity address used by the device, which can be a public
       * or random static device address.
       *
       * @param[out] address Bluetooth public address in little endian format
       * @param[out] type Address type   */
      sc = sl_bt_system_get_identity_address(&(bleData->myAddress),
                                             &(bleData->myAddressType));
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_system_get_identity_address() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

#if DEVICE_IS_BLE_SERVER

      //for server only

      /*Create an advertising set. The handle of the created advertising set is
       * returned in response.
       *
       * @param[out] handle Advertising set handle  */
      sc = sl_bt_advertiser_create_set(&(bleData->advertisingSetHandle));
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_create_set() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /*
       * Set the advertising timing parameters of the given advertising set. This
       * setting will take effect next time that advertising is enabled.
       *
       * @param[in] handle Advertising set handle
       * @param[in] interval_min - Minimum advertising interval. Value in units of 0.625 ms
       * @param[in] interval_max - Maximum advertising interval. Value in units of 0.625 ms */
      sc = sl_bt_advertiser_set_timing(bleData->advertisingSetHandle,
                                       adv_int,
                                       adv_int,
                                       0,
                                       0);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_set_timing() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /*
       * Start advertising of a given advertising set with specified discoverable and
       * connectable modes.
       *
       * @param[in] handle Advertising set handle
       * @param[in] discover Enum @ref sl_bt_advertiser_discoverable_mode_t.
       *   Discoverable mode. sl_bt_advertiser_general_discoverable - Discoverable using
       *       general discovery procedure
       * @param[in] connect Enum @ref sl_bt_advertiser_connectable_mode_t. Connectable
       *   mode. sl_bt_advertiser_connectable_scannable - Undirected
       *       connectable scannable. */
      sc = sl_bt_advertiser_start(bleData->advertisingSetHandle,
                                  sl_bt_advertiser_general_discoverable,
                                  sl_bt_advertiser_connectable_scannable);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      displayPrintf(DISPLAY_ROW_NAME, "Server");
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");

#else
      //for client only

      /* Set the scan mode on the specified PHYs. If the device is currently scanning
       for advertising devices on PHYs, new parameters will take effect when
       scanning is restarted.

       @param[in] phys PHYs for which the parameters are set.
       @param[in] scan_mode @parblock
         Scan mode. Values:
           - <b>0:</b> Passive scanning
           - <b>1:</b> Active scanning    */
      sc = sl_bt_scanner_set_mode(sl_bt_gap_1m_phy, SCAN_PASSIVE);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_scanner_set_mode() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /* Set the scanning timing parameters on the specified PHYs. If the device is
         currently scanning for advertising devices on PHYs, new parameters will take
          effect when scanning is restarted.

        @param[in] phys PHYs for which the parameters are set.
        @param[in] scan_interval @parblock
           Scan interval is defined as the time interval when the device starts its
            last scan until it begins the subsequent scan. In other words, how often to
            scan

        @param[in] scan_window @parblock
        Scan window defines the duration of the scan which must be less than or
        equal to the @p scan_interval */
      sc = sl_bt_scanner_set_timing(sl_bt_gap_1m_phy, scan_int, scan_window);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_scanner_set_timing() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /* Set the default Bluetooth connection parameters. The values are valid for all
         subsequent connections initiated by this device. To change parameters of an
       already established connection, use the command @ref
       sl_bt_connection_set_parameters.   */
      sc = sl_bt_connection_set_default_parameters(conn_int,
                                                   conn_int,
                                                   slave_latency,
                                                   spvsn_timeout,
                                                   0,
                                                   4);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_scanner_set_default_parameters() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /*  Start the GAP discovery procedure to scan for advertising devices on the
          specified scanning PHYs. To cancel an ongoing discovery procedure, use the
          @ref sl_bt_scanner_stop command.

          @param[in] scanning_phy @parblock
          The scanning PHYs.

            @param[in] discover_mode Enum @ref sl_bt_scanner_discover_mode_t. Bluetooth
          discovery Mode. Values:
      - <b>sl_bt_scanner_discover_limited (0x0):</b> Discover only limited
        discoverable devices.
      - <b>sl_bt_scanner_discover_generic (0x1):</b> Discover limited and
        generic discoverable devices.
      - <b>sl_bt_scanner_discover_observation (0x2):</b> Discover all devices.  */

      sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      displayPrintf(DISPLAY_ROW_NAME, "Client");
      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");

#endif

      //Configuration according to constants set at compile time.
      sc = sl_bt_sm_configure(FLAGS, sl_bt_sm_io_capability_displayyesno);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_sm_configure() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      //delete bonding data from server
      sc = sl_bt_sm_delete_bondings();
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_sm_delete_bondings() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                    bleData->myAddress.addr[0],
                    bleData->myAddress.addr[1],
                    bleData->myAddress.addr[2],
                    bleData->myAddress.addr[3],
                    bleData->myAddress.addr[4],
                    bleData->myAddress.addr[5]);
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "Final Project");

      //initialize connection and indication flags
      bleData->connected           = false;
      bleData->bonded           = false;
      bleData->indication          = false;
      bleData->button_indication   = false;
      bleData->gesture_indication   = false;
      bleData->oximeter_indication   = false;
      bleData->indication_inFlight = false;
      bleData->button_pressed = false;
      bleData->pb1_button_pressed = false;
      bleData->gesture_value = 0x00;
      bleData->gesture_on = false;
      bleData->oximeter_off = true;

      break;

      //Indicates that a new connection was opened
    case sl_bt_evt_connection_opened_id:

      //LOG_INFO("Connection opened event\n\r");
      //set connection flag as true
      bleData->connected         = true;
      //get value of connection handle
      /*PACKSTRUCT( struct sl_bt_evt_connection_opened_s
            {
              bd_addr address;       Remote device address
              uint8_t address_type;
              uint8_t master;
              uint8_t connection;
              uint8_t bonding;
              uint8_t advertiser;   */
      bleData->connection_handle = evt->data.evt_connection_opened.connection;

#if DEVICE_IS_BLE_SERVER

      //for server only

      /*
       * Stop the advertising of the given advertising set.
       * @param[in] handle Advertising set handle */
      sc = sl_bt_advertiser_stop(bleData->advertisingSetHandle);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      //set parameter values from server
      sc = sl_bt_connection_set_parameters(bleData->connection_handle,
                                           conn_int,
                                           conn_int,
                                           slave_latency,
                                           spvsn_timeout,
                                           0,
                                           0);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_connection_set_parameters() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

#else

      //display server address on client
      displayPrintf(DISPLAY_ROW_BTADDR2, "%02X:%02X:%02X:%02X:%02X:%02X",
                    server_addr[0],
                    server_addr[1],
                    server_addr[2],
                    server_addr[3],
                    server_addr[4],
                    server_addr[5]);

#endif

      displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

      break;

      //Indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:

      //LOG_INFO("connection close event\n\r");

      gpioLed0SetOff();
      gpioLed1SetOff();

      displayPrintf(DISPLAY_ROW_8,"");
      displayPrintf(DISPLAY_ROW_9, "");
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");

      //turn off all connection and indication flags
      bleData->connected           = false;
      bleData->bonded           = false;
      bleData->indication          = false;
      bleData->button_indication   = false;
      bleData->gesture_indication   = false;
      bleData->oximeter_indication   = false;
      bleData->indication_inFlight = false;
      bleData->button_pressed = false;
      bleData->pb1_button_pressed = false;
      bleData->gesture_value = 0x00;
      bleData->gesture_on = false;
      bleData->oximeter_off = true;

      //delete bonding data from server
      sc = sl_bt_sm_delete_bondings();
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_sm_delete_bondings() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

#if DEVICE_IS_BLE_SERVER

      //for server only

      //Start advertising of a given advertising set with specified discoverable and connectable modes.
      sc = sl_bt_advertiser_start(bleData->advertisingSetHandle,
                                  sl_bt_advertiser_general_discoverable,
                                  sl_bt_advertiser_connectable_scannable);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");

#else
      sc = sl_bt_scanner_start(sl_bt_gap_1m_phy, sl_bt_scanner_discover_generic);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_scanner_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }
      displayPrintf(DISPLAY_ROW_CONNECTION, "Discovering");

#endif

      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
      displayPrintf(DISPLAY_ROW_BTADDR2, "");
      break;

      //Indicates a user request to display that the new bonding request is
      // received and for the user to confirm the request
    case sl_bt_evt_sm_confirm_bonding_id:

      //LOG_INFO("Confirm bonding event\n\r");
      //Accept or reject the bonding request.
      sc = sl_bt_sm_bonding_confirm(bleData->connection_handle, 1);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_sm_bonding_confirm() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      break;

      //Indicates a request for passkey display and confirmation by the user
    case sl_bt_evt_sm_confirm_passkey_id:

      //LOG_INFO("Confirm passkey event\n\r");
      //get the passkey
      bleData->passkey = evt->data.evt_sm_confirm_passkey.passkey;

      displayPrintf(DISPLAY_ROW_PASSKEY, "%d", bleData->passkey);
      displayPrintf(DISPLAY_ROW_ACTION, "Confirm with PB0");

      break;

      //Triggered after the pairing or bonding procedure is successfully completed.
    case sl_bt_evt_sm_bonded_id:

      //LOG_INFO("Bonded event\n\r");
      displayPrintf(DISPLAY_ROW_CONNECTION, "Bonded");
      bleData->bonded           = true;
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");

      break;

      // This event is triggered if the pairing or bonding procedure fails.
    case sl_bt_evt_sm_bonding_failed_id:

      LOG_ERROR("Bonding failed reason=0x%04x\n\r", evt->data.evt_sm_bonding_failed.reason);

      //close connection
      sc = sl_bt_connection_close(bleData->connection_handle);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_connection_close() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      break;

      //Triggered whenever the connection parameters are changed and at any time a connection is established
    case sl_bt_evt_connection_parameters_id:

#if LOG_PARAMETER_VALUES
      /*PACKSTRUCT( struct sl_bt_evt_connection_parameters_s
      {
        uint8_t  connection;
        uint16_t interval;
        uint16_t latency;
        uint16_t timeout;
       */
      LOG_INFO("Connection params: connection=%d, interval=%d, latency=%d, timeout=%d, securitymode=%d\n\r",
               (int) (evt->data.evt_connection_parameters.connection),
               (int) (evt->data.evt_connection_parameters.interval*1.25),
               (int) (evt->data.evt_connection_parameters.latency),
               (int) (evt->data.evt_connection_parameters.timeout*10),
               (int) (evt->data.evt_connection_parameters.security_mode) );
      //log parameters value
#endif
      break;

      //Indicates that the external signals have been received
    case sl_bt_evt_system_external_signal_id:

      //check for button pressed event
      if(evt->data.evt_system_external_signal.extsignals == evt_ButtonPressed) {

#if DEVICE_IS_BLE_SERVER

          bool ret;

          //displayPrintf(DISPLAY_ROW_9, "Button Pressed");

          if(bleData->pb1_button_pressed) {
              LOG_INFO("Enabling gesture sensor\n\r");

              displayPrintf(DISPLAY_ROW_10, "Enable gesture sensor");

              ret = SparkFun_APDS9960_init();
              if(ret != true) {
                  LOG_ERROR("Error initializing APDS\n\r");
              }
              else {
                  //LOG_INFO("APDS initialized\n\r");
              }

              ret = enableGestureSensor(true);
              if(ret != true) {
                  LOG_ERROR("Error enabling gesture\n\r");
              }
              else {
                  //LOG_INFO("gesture enabled\n\r");
                  displayPrintf(DISPLAY_ROW_10, "Gesture sensor ON");
                  bleData->gesture_on = true;
                  bleData->gesture_value = 0x00;

              }

          }

#endif

          //if PB1 is pressed, read button state service characteristic value
#if !DEVICE_IS_BLE_SERVER
          if(bleData->pb1_button_pressed) {
              //LOG_INFO("PB1 button pressed\n\r");
              sc = sl_bt_gatt_read_characteristic_value(bleData->connection_handle,
                                                        bleData->button_char_handle);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_read_characteristic_value() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
          }

#endif

          if(bleData->button_pressed && bleData->bonded == false) {

              //LOG_INFO("sends passkey confirm?\n\r");
              //Accept or reject the reported passkey confirm value.
              sc = sl_bt_sm_passkey_confirm(bleData->connection_handle, 1);

              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_sm_passkey_confirm() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
          }
      }

      break;

      //Indicates that a soft timer has lapsed.
    case sl_bt_evt_system_soft_timer_id:

      displayUpdate();

      break;


#if DEVICE_IS_BLE_SERVER

      //for servers
      /*Indicates either that a local Client Characteristic Configuration
      descriptor was changed by the remote GATT client, or that a confirmation from
      the remote GATT client was received upon a successful reception of the
      indication */
    case sl_bt_evt_gatt_server_characteristic_status_id:

      //check if temperature measurement characteristic is changed
      /*PACKSTRUCT( struct sl_bt_evt_gatt_server_characteristic_status_s
      {
        uint8_t  connection;           Connection handle
        uint16_t characteristic;       GATT characteristic handle. This value is
                                           normally received from the
                                           gatt_characteristic event.
        uint8_t  status_flags;         Describes whether Client Characteristic
                                           Configuration was changed or if a
                                           confirmation was received.
        uint16_t client_config_flags; This field carries the new value of the
                                           Client Characteristic Configuration.
        uint16_t client_config;       The handle of client-config descriptor. */

      //check if the characteristic change is for HTM
      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_gesture_state) {

          //check if any status flag has been changed by client
          if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)
              evt->data.evt_gatt_server_characteristic_status.status_flags) {

              //check if indication flag is disabled
              if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_disable) {
                  //bleData->gesture_indication = false;
                  //gpioLed0SetOff();
                  displayPrintf(DISPLAY_ROW_9, "");
                  //LOG_INFO("gesture indication off\n\r");

              }

              //check if indication flag is enabled
              else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication) {
                  //bleData->gesture_indication = true;
                  //gpioLed0SetOn();
                  //LOG_INFO("gesture indication on\n\r");
              }

          }

      }

      //check if the characteristic change is from Push button
      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_oximeter_state) {

          //check if any status flag has been changed by client
          if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)
              evt->data.evt_gatt_server_characteristic_status.status_flags) {

              //check if indication flag is disabled
              if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_disable) {
                  //bleData->oximeter_indication = false;
                  //gpioLed1SetOff();
                  //LOG_INFO("oximeter indication off\n\r");

              }

              //check if indication flag is enabled
              else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication) {
                  //bleData->oximeter_indication = true;
                  //gpioLed1SetOn();
                  //LOG_INFO("oximeter indication on\n\r");
              }

          }

      }

      //check if indication confirmation has been received from client
      if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)
          evt->data.evt_gatt_server_characteristic_status.status_flags) {
          //LOG_INFO("Received indication confirmation\n\r");
          bleData->indication_inFlight = false;
      }

      break;

      //event indicates confirmation from the remote GATT client has not been received within 30 seconds after an indication was sent
    case sl_bt_evt_gatt_server_indication_timeout_id:

      LOG_ERROR("server indication timeout\n\r");
      bleData->indication = false;
      bleData->button_indication = false;
      break;

#else
      //for clients

      // Reports an advertising or scan response packet that is received by the
      // device's radio while in scanning mode
    case sl_bt_evt_scanner_scan_report_id:

      // Parse advertisement packets
      if (evt->data.evt_scanner_scan_report.packet_type == 0) {
          //check whether it is desired server
          if((evt->data.evt_scanner_scan_report.address.addr[0] == server_addr[0]) &&
              (evt->data.evt_scanner_scan_report.address.addr[1] == server_addr[1]) &&
              (evt->data.evt_scanner_scan_report.address.addr[2] == server_addr[2]) &&
              (evt->data.evt_scanner_scan_report.address.addr[3] == server_addr[3]) &&
              (evt->data.evt_scanner_scan_report.address.addr[4] == server_addr[4]) &&
              (evt->data.evt_scanner_scan_report.address.addr[5] == server_addr[5]) &&
              (evt->data.evt_scanner_scan_report.address_type==0)) {

              //stop scanner
              sc = sl_bt_scanner_stop();
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_scanner_stop() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }

              /*  @param[in] address Address of the device to connect to
          @param[in] address_type Enum @ref sl_bt_gap_address_type_t. Address type of
          the device to connect to.
          @param[in] initiating_phy Enum @ref sl_bt_gap_phy_t. The initiating PHY.
          @param[out] connection Handle that will be assigned to the connection after
          the connection is established. This handle is valid only if the result code
            of this response is 0 (zero).  */
              sc = sl_bt_connection_open(evt->data.evt_scanner_scan_report.address,
                                         evt->data.evt_scanner_scan_report.address_type, sl_bt_gap_1m_phy, NULL);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_connection_open() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
          }
      }

      break;

      //Indicates that the current GATT procedure was completed successfully
      // or that it failed with an error
    case sl_bt_evt_gatt_procedure_completed_id:

      //error code checking when PB1 is pressed on client for first time
      if(evt->data.evt_gatt_procedure_completed.result == 0x110F) {

          //increase security(numeric comparison)
          sc = sl_bt_sm_increase_security(bleData->connection_handle);
          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_sm_increase_security() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
      }

      break;

      // Indicate that a GATT service in the remote GATT database was
      // discovered
    case sl_bt_evt_gatt_service_id:

      //LOG_INFO("Service id event\n\r");

      //memcpy(complete_thermo_service_uuid, thermo_service, sizeof(thermo_service));
      //memcpy(complete_button_service_uuid, button_service, sizeof(thermo_service));

      /* PACKSTRUCT( struct sl_bt_evt_gatt_service_s
{
  uint8_t    connection; < Connection handle
  uint32_t   service;    < GATT service handle
  uint8array uuid;       < Service UUID in little endian format
}); */
      //save service handle
      if(memcmp(evt->data.evt_gatt_service.uuid.data, thermo_service, sizeof(thermo_service)) == 0)
        bleData->service_handle = evt->data.evt_gatt_service.service;
      else if(memcmp(evt->data.evt_gatt_service.uuid.data, button_service, sizeof(button_service)) == 0) {
          //LOG_INFO("memcmp button_service\n\r");
        bleData->button_service_handle = evt->data.evt_gatt_service.service;
      }
      else if(memcmp(evt->data.evt_gatt_service.uuid.data, gesture_service, sizeof(gesture_service)) == 0) {
          //LOG_INFO("memcmp gesture_service\n\r");
              bleData->gesture_service_handle = evt->data.evt_gatt_service.service;
      }
      else if(memcmp(evt->data.evt_gatt_service.uuid.data, oximeter_service, sizeof(oximeter_service)) == 0)
              bleData->oximeter_service_handle = evt->data.evt_gatt_service.service;
      break;

      // Indicates that a GATT characteristic in the remote GATT database was
      // discovered
    case sl_bt_evt_gatt_characteristic_id:

      //LOG_INFO("Characteristic id event\n\r");
      // memcpy(complete_thermo_char_uuid, thermo_char, sizeof(thermo_char));
      //  memcpy(complete_button_char_uuid, button_char, sizeof(thermo_char));
      /* PACKSTRUCT( struct sl_bt_evt_gatt_characteristic_s
{
  uint8_t    connection;     < Connection handle
  uint16_t   characteristic; < GATT characteristic handle
  uint8_t    properties;     < Characteristic properties
  uint8array uuid;           < Characteristic UUID in little endian format
});*/
      //save characteristic handle
      if(memcmp(evt->data.evt_gatt_characteristic.uuid.data, thermo_char, sizeof(thermo_char)) == 0)
        bleData->char_handle = evt->data.evt_gatt_characteristic.characteristic;
      else if(memcmp(evt->data.evt_gatt_characteristic.uuid.data, button_char, sizeof(button_char)) == 0)
        bleData->button_char_handle = evt->data.evt_gatt_characteristic.characteristic;
      else if(memcmp(evt->data.evt_gatt_characteristic.uuid.data, gesture_char, sizeof(gesture_char)) == 0)
              bleData->gesture_char_handle = evt->data.evt_gatt_characteristic.characteristic;
      else if(memcmp(evt->data.evt_gatt_characteristic.uuid.data, oximeter_char, sizeof(oximeter_char)) == 0)
              bleData->oximeter_char_handle = evt->data.evt_gatt_characteristic.characteristic;
      break;

      // Indicates that the value of one or several characteristics in the
      // remote GATT server was received
    case sl_bt_evt_gatt_characteristic_value_id:

      LOG_INFO("GATT char value event client\n\r");

      /*PACKSTRUCT( struct sl_bt_evt_gatt_characteristic_value_s
      {
        uint8_t    connection;     < Connection handle
        uint16_t   characteristic; < GATT characteristic handle. This value is
                                        normally received from the gatt_characteristic
                                        event.
        uint8_t    att_opcode;     < Enum @ref sl_bt_gatt_att_opcode_t. Attribute
                                        opcode, which indicates the GATT transaction
                                        used.
        uint16_t   offset;         < Value offset
        uint8array value;          < Characteristic value
      }); */
      //send indication confirmation to server, sl_bt_gatt_handle_value_indication

      //check if we got an indication, if yes send a confirmation to server
      if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_handle_value_indication) {
          sc = sl_bt_gatt_send_characteristic_confirmation(bleData->connection_handle);
          //LOG_INFO("got indication, sending confirmation\n\r");

          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }

          if(evt->data.evt_gatt_characteristic_value.characteristic == bleData->char_handle) {
              //save value got from server in a variable
              bleData->char_value = &(evt->data.evt_gatt_characteristic_value.value.data[0]);

              temperature_in_c = FLOAT_TO_INT32((bleData->char_value));
              displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%d", temperature_in_c);
          }

          if(evt->data.evt_gatt_characteristic_value.characteristic == bleData->gesture_char_handle) {
                    //LOG_INFO("Got gesture indication\n\r");
                          if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
                              bleData->gesture_value = 0x01;
                              displayPrintf(DISPLAY_ROW_9, "Gesture = LEFT");
                          }
                          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x02){
                              bleData->gesture_value = 0x02;
                              displayPrintf(DISPLAY_ROW_9, "Gesture = RIGHT");
                          }
                          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x03){
                              bleData->gesture_value = 0x03;
                                              displayPrintf(DISPLAY_ROW_9, "Gesture = UP");
                                          }
                          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x04){
                                              displayPrintf(DISPLAY_ROW_9, "Gesture = DOWN");
                                          }
                          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x05){
                                              displayPrintf(DISPLAY_ROW_9, "Gesture = NEAR");
                                          }
                          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x06){
                                              displayPrintf(DISPLAY_ROW_9, "Gesture = FAR");
                                          }
                          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
                              displayPrintf(DISPLAY_ROW_9, "Gesture = NONE");
                          }

                          if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x04){
                              displayPrintf(DISPLAY_ROW_10, "Gesture sensor OFF");
                          }
                          else {
                              displayPrintf(DISPLAY_ROW_10, "Gesture sensor ON");
                          }
                }
      }

      if(evt->data.evt_gatt_characteristic_value.characteristic == bleData->oximeter_char_handle) {

              //LOG_INFO("Does it get oximeter indications?\n\r");

              if(bleData->gesture_value == 0x01){
                  displayPrintf(DISPLAY_ROW_TEMPVALUE, "Oxygen level: %d", evt->data.evt_gatt_characteristic_value.value.data[0]);
              }

              else if(bleData->gesture_value == 0x02){
                  displayPrintf(DISPLAY_ROW_TEMPVALUE, "Heart rate: %d", evt->data.evt_gatt_characteristic_value.value.data[0]);
              }
      }



      //check if we got a read response, if yes show button on lcd
     /* if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response) {
          if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
              displayPrintf(DISPLAY_ROW_9, "Start gesture");
          }
          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
              displayPrintf(DISPLAY_ROW_9, "Button Released");
          }
      }

      //display button state on client
      if(bleData->bonded && bleData->button_indication && (evt->data.evt_gatt_characteristic_value.characteristic == bleData->button_char_handle)) {
          if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
              displayPrintf(DISPLAY_ROW_9, "Start gesture");
          }
          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
              displayPrintf(DISPLAY_ROW_9, "Button Released");
          }
      }*/


      break;

#endif

  }
}
