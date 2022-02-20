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
//function to increment read and write pointer in circular buffer
int inc_ptr(int ptr) {

  //reset the pointer value after reaching buffer limit
  if(ptr+1 >= MAX_PTR) {
      ptr=0;
  }

  //increment the pointer by one
  else {
      ptr++;
  }

  return ptr;
}

//function to store indication data in circular buffer
//@para: [struct buffer_entry write_data]
//        structure with all the indication data
int enqueue(struct buffer_entry write_data) {

  ble_data_struct_t *bleData = getBleDataPtr();

  //check if the buffer is full
  if(bleData->full) {
      LOG_ERROR("Buffer is full\n\r");
      return -1;
  }

  //store the indication data in the circular buffer at location pointed by wptr(write pointer)
  bleData->indication_buffer[bleData->wptr] = write_data;

  //increment write pointer
  bleData->wptr = inc_ptr(bleData->wptr);

  //if wptr and rptr are at same location, buffer is full
  if(bleData->wptr == bleData->rptr)
    bleData->full = 1;

  return 0;

}

//function to dequeue indication data from the circular buffer and send  the indication
int dequeue() {

  ble_data_struct_t *bleData = getBleDataPtr();

  //check if buffer is empty
  if(bleData->wptr==bleData->rptr && !bleData->full) {
      LOG_ERROR("Buffer is empty\n\r");
      return -1;
  }

  struct buffer_entry read_data;

  //get indication data from the circular buffer
  read_data = bleData->indication_buffer[bleData->rptr];

  //send the indication to client
  sc = sl_bt_gatt_server_send_indication(bleData->connection_handle,
                                         read_data.charHandle,
                                         read_data.bufferLength,
                                         &(read_data.buffer[0]));

  //check indication return status
  if(sc != SL_STATUS_OK) {
      LOG_ERROR("sl_bt_gatt_server_send_indication() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      return -1;
  }
  else {

      //if buffer was previously full, reset it since we read an element from there
      if(bleData->full)
        bleData->full=false;

      //increment the read pointer
      bleData->rptr = inc_ptr(bleData->rptr);

      //indication is sent i.e. indication is in flight
      bleData->indication_inFlight = true;
      //decrement queued indications
      bleData->queued_indication--;

      return 0;
  }


}

//function to send temperature value indication to client
void ble_SendTemp() {

  uint8_t htm_temperature_buffer[5];
  uint8_t *p = htm_temperature_buffer;
  uint32_t htm_temperature_flt;
  uint8_t flags = 0x00;
  ble_data_struct_t *bleData = getBleDataPtr();

  struct buffer_entry indication_data;

  //check if bluetooth is connected
  if(bleData->connected == true){

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

      //check if indication is on
      if (bleData->indication == true) {

          //check if any indication is inFlight
          if(bleData->indication_inFlight) {

              //save the characteristic values in a buffer_entry
              indication_data.charHandle = gattdb_temperature_measurement;
              indication_data.bufferLength = 5;
              for(int i=0; i<5; i++)
                indication_data.buffer[i] = htm_temperature_buffer[i];

              //store indication data in circular buffer
              qc = enqueue(indication_data);

              //increase number of queued indications
              if(qc == 0)
                bleData->queued_indication++;
              else
                LOG_ERROR("Indication enqueue failed\n\r");
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
                  //LOG_INFO("Sent HTM indication, temp=%f\n\r", temperature_in_c);
                  displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%f", temperature_in_c);
              }
          }
      }
  }

}

//function to send button state indication
void ble_SendButtonState(uint8_t value) {

  ble_data_struct_t *bleData = getBleDataPtr();

  struct buffer_entry indication_data;

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
      if (bleData->button_indication == true && bleData->bonded == true) {

          //check if any indication is inFlight
          if(bleData->indication_inFlight) {

              //save the characteristic values in a buffer_entry
              indication_data.charHandle = gattdb_button_state;
              indication_data.bufferLength = 2;
              for(int i=0; i<2; i++)
                indication_data.buffer[i] = button_value_buffer[i];

              //store indication data in circular buffer
              qc = enqueue(indication_data);

              //increase number of queued indications
              if(qc == 0)
                bleData->queued_indication++;
              else
                LOG_ERROR("Indication enqueue failed\n\r");
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

  /*uint8_t complete_thermo_service_uuid[2];
  uint8_t complete_thermo_char_uuid[2];
  uint8_t complete_button_service_uuid[16];
  uint8_t complete_button_char_uuid[16];*/

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
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A9");

      //initialize connection and indication flags
      bleData->connected           = false;
      bleData->bonded           = false;
      bleData->indication          = false;
      bleData->button_indication   = false;
      bleData->indication_inFlight = false;
      bleData->gatt_procedure      = false;
      bleData->rptr = 0;
      bleData->wptr = 0;
      bleData->queued_indication = 0;
      bleData->button_pressed = false;
      bleData->pb1_button_pressed = false;
      bleData->press_seq = 0;

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

      displayPrintf(DISPLAY_ROW_9, "");
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
      displayPrintf(DISPLAY_ROW_PASSKEY, "");
      displayPrintf(DISPLAY_ROW_ACTION, "");

      //turn off all connection and indication flags
      bleData->connected           = false;
      bleData->bonded           = false;
      bleData->indication          = false;
      bleData->button_indication   = false;
      bleData->indication_inFlight = false;
      bleData->gatt_procedure      = false;
      bleData->rptr = 0;
      bleData->wptr = 0;
      bleData->queued_indication = 0;
      bleData->button_pressed = false;
      bleData->pb1_button_pressed = false;
      bleData->press_seq = 0;

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

#if !DEVICE_IS_BLE_SERVER

      //detect button press sequence to change indication status for button state service
      //check if PB0 is pressed once
      if(evt->data.evt_system_external_signal.extsignals == evt_ButtonPressed && bleData->button_pressed)
        bleData->press_seq = 1;

      //check if PB1 is pressed while PB0 is pressed, if yes break from the loop
      if((bleData->press_seq == 1) && bleData->pb1_button_pressed && evt->data.evt_system_external_signal.extsignals == evt_ButtonPressed) {
          bleData->press_seq = 2;
          //break;
      }

      //check if PB1 is released in sequence, if yes break from the loop
      if((bleData->press_seq == 2) && !bleData->pb1_button_pressed && evt->data.evt_system_external_signal.extsignals == evt_ButtonReleased) {
          bleData->press_seq = 3;
          break;
      }

      //check if PB0 is released in sequence, if yes break from the loop after toggling indication status for button state
      if((bleData->press_seq == 3) && !bleData->button_pressed && evt->data.evt_system_external_signal.extsignals == evt_ButtonReleased) {
          if(bleData->button_indication) {
              sc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                              bleData->button_char_handle,
                                                              sl_bt_gatt_disable);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_set_characteristic_notification() 1 returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
              else {
                  // LOG_INFO("Disables indications from client\n\r");
              }
          }
          else {
              sc = sl_bt_gatt_set_characteristic_notification(bleData->connection_handle,
                                                              bleData->button_char_handle,
                                                              sl_bt_gatt_indication);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_set_characteristic_notification() 2 returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
              else {
                  // LOG_INFO("Enables indications from client\n\r");
              }
          }
          break;

      }

#endif

      //check for button pressed event
      if(evt->data.evt_system_external_signal.extsignals == evt_ButtonPressed) {

#if DEVICE_IS_BLE_SERVER

          displayPrintf(DISPLAY_ROW_9, "Button Pressed");

          if(bleData->bonded)
            ble_SendButtonState(0x01);


#endif

          //if PB1 is pressed, read button state service characteristic value
#if !DEVICE_IS_BLE_SERVER
          if(bleData->pb1_button_pressed) {
              sc = sl_bt_gatt_read_characteristic_value(bleData->connection_handle,
                                                        bleData->button_char_handle);
              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_gatt_read_characteristic_value() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
          }

#endif

          if(bleData->button_pressed && bleData->bonded == false) {

              //Accept or reject the reported passkey confirm value.
              sc = sl_bt_sm_passkey_confirm(bleData->connection_handle, 1);

              if(sc != SL_STATUS_OK) {
                  LOG_ERROR("sl_bt_sm_passkey_confirm() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
              }
          }
      }

#if DEVICE_IS_BLE_SERVER
      //check for button released event
      if(evt->data.evt_system_external_signal.extsignals == evt_ButtonReleased) {

          displayPrintf(DISPLAY_ROW_9, "Button Released");

          if(bleData->bonded)
            ble_SendButtonState(0x00);
      }


#endif

      break;

      //Indicates that a soft timer has lapsed.
    case sl_bt_evt_system_soft_timer_id:

      displayUpdate();

#if DEVICE_IS_BLE_SERVER
      //check for any indications queued or if any indication is inFlight
      if(bleData->queued_indication!=0 && !bleData->indication_inFlight) {

          //read data and send indication
          qc = dequeue();

          if(qc != 0)
            LOG_ERROR("Indication dequeue failed\n\r");

      }



#endif

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
      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) {

          //check if any status flag has been changed by client
          if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)
              evt->data.evt_gatt_server_characteristic_status.status_flags) {

              //check if indication flag is disabled
              if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_disable) {
                  bleData->indication = false;
                  gpioLed0SetOff();
                  displayPrintf(DISPLAY_ROW_TEMPVALUE, "");

              }

              //check if indication flag is enabled
              else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication) {
                  bleData->indication = true;
                  gpioLed0SetOn();
              }

          }

      }

      //check if the characteristic change is from Push button
      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_button_state) {

          //check if any status flag has been changed by client
          if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)
              evt->data.evt_gatt_server_characteristic_status.status_flags) {

              //check if indication flag is disabled
              if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_disable) {
                  bleData->button_indication = false;
                  gpioLed1SetOff();

              }

              //check if indication flag is enabled
              else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication) {
                  bleData->button_indication = true;
                  gpioLed1SetOn();
              }

          }

      }

      //check if indication confirmation has been received from client
      if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)
          evt->data.evt_gatt_server_characteristic_status.status_flags) {
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

      //toggle indication state if required button press sequence detected
      if(bleData->press_seq == 3) {
          bleData->press_seq = 0;
          bleData->button_indication = !bleData->button_indication;
      }

      bleData->gatt_procedure = false;

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
      else if(memcmp(evt->data.evt_gatt_service.uuid.data, button_service, sizeof(button_service)) == 0)
        bleData->button_service_handle = evt->data.evt_gatt_service.service;
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
      break;

      // Indicates that the value of one or several characteristics in the
      // remote GATT server was received
    case sl_bt_evt_gatt_characteristic_value_id:

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

          if(sc != SL_STATUS_OK) {
              LOG_ERROR("sl_bt_gatt_set_characteristic_notification() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
          }
      }

      if(evt->data.evt_gatt_characteristic_value.characteristic == bleData->char_handle) {
          //save value got from server in a variable
          bleData->char_value = &(evt->data.evt_gatt_characteristic_value.value.data[0]);

          temperature_in_c = FLOAT_TO_INT32((bleData->char_value));
          displayPrintf(DISPLAY_ROW_TEMPVALUE, "Temp=%d", temperature_in_c);
      }

      //check if we got a read response, if yes show button on lcd
      if(evt->data.evt_gatt_characteristic_value.att_opcode == sl_bt_gatt_read_response) {
          if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
              displayPrintf(DISPLAY_ROW_9, "Button Pressed");
          }
          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
              displayPrintf(DISPLAY_ROW_9, "Button Released");
          }
      }

      //display button state on client
      if(bleData->bonded && bleData->button_indication && (evt->data.evt_gatt_characteristic_value.characteristic == bleData->button_char_handle)) {
          if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x01) {
              displayPrintf(DISPLAY_ROW_9, "Button Pressed");
          }
          else if(evt->data.evt_gatt_characteristic_value.value.data[0] == 0x00){
              displayPrintf(DISPLAY_ROW_9, "Button Released");
          }
      }

      break;

#endif

  }
}
