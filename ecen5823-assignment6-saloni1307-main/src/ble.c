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

#define LOG_PARAMETER_VALUES 0

// BLE private data
ble_data_struct_t ble_data;

sl_status_t sc=0;

uint32_t adv_int=0x190;           //250 ms advertisement interval
uint16_t conn_int = 0x3c;         //75 ms connection interval
uint16_t slave_latency = 0x03;    //3 slave latency - slave can skip upto 3 connection events
uint16_t spvsn_timeout = 0x50;   //800ms supervision timeout

ble_data_struct_t * getBleDataPtr() {

  return (&ble_data);
}

//function to send temperature value indication to client
void ble_SendTemp() {

  uint8_t htm_temperature_buffer[5];
  uint8_t *p = htm_temperature_buffer;
  uint32_t htm_temperature_flt;
  uint8_t flags = 0x00;
  ble_data_struct_t *bleData = getBleDataPtr();

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
      if (bleData->indication == true && bleData->indication_inFlight == false) {

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

//function to handle BLE events
void handle_ble_event(sl_bt_msg_t *evt) {

  ble_data_struct_t *bleData = getBleDataPtr();

  //check ble event
  switch(SL_BT_MSG_ID(evt->header)) {

    //for both server and client

    //system boot event
    //Indicates that the device has started and the radio is ready
    case sl_bt_evt_system_boot_id:


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

      displayInit();
      displayPrintf(DISPLAY_ROW_NAME, "Server");
      displayPrintf(DISPLAY_ROW_BTADDR, "%02X:%02X:%02X:%02X:%02X:%02X",
                    bleData->myAddress.addr[0],
                    bleData->myAddress.addr[1],
                    bleData->myAddress.addr[2],
                    bleData->myAddress.addr[3],
                    bleData->myAddress.addr[4],
                    bleData->myAddress.addr[5]);
      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
      displayPrintf(DISPLAY_ROW_ASSIGNMENT, "A6");

      //initialize connection and indication flags
      bleData->connected           = false;
      bleData->indication          = false;
      bleData->indication_inFlight = false;
      break;

      //Indicates that a new connection was opened
    case sl_bt_evt_connection_opened_id:

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

      /*
       * Stop the advertising of the given advertising set.
       * @param[in] handle Advertising set handle */
      sc = sl_bt_advertiser_stop(bleData->advertisingSetHandle);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_stop() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      /*PACKSTRUCT( struct sl_bt_evt_connection_opened_s
      {
       bd_addr address;
       uint8_t address_type;
       uint8_t master;
       uint8_t connection;
       uint8_t bonding;
       uint8_t advertiser;
      });*/
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
      displayPrintf(DISPLAY_ROW_CONNECTION, "Connected");

      break;

      //Indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:

      //turn off all connection and indication flags
      bleData->connected           = false;
      bleData->indication          = false;
      bleData->indication_inFlight = false;

      //Start advertising of a given advertising set with specified discoverable and connectable modes.
      sc = sl_bt_advertiser_start(bleData->advertisingSetHandle,
                                  sl_bt_advertiser_general_discoverable,
                                  sl_bt_advertiser_connectable_scannable);
      if(sc != SL_STATUS_OK) {
          LOG_ERROR("sl_bt_advertiser_start() returned != 0 status=0x%04x\n\r", (unsigned int)sc);
      }

      displayPrintf(DISPLAY_ROW_CONNECTION, "Advertising");
      displayPrintf(DISPLAY_ROW_TEMPVALUE, "");
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

      break;

    case sl_bt_evt_system_soft_timer_id:

      displayUpdate();

      break;

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
      if(evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_temperature_measurement) {

          //check if any status flag has been changed by client
          if (sl_bt_gatt_server_client_config == (sl_bt_gatt_server_characteristic_status_flag_t)
              evt->data.evt_gatt_server_characteristic_status.status_flags) {

              //check if indication flag is disabled
              if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_disable) {
                  bleData->indication = false;
                  displayPrintf(DISPLAY_ROW_TEMPVALUE, "");

              }

              //check if indication flag is enabled
              else if(evt->data.evt_gatt_server_characteristic_status.client_config_flags == gatt_indication) {
                  bleData->indication = true;
              }

          }

          //check if indication confirmation has been received from client
          if (sl_bt_gatt_server_confirmation == (sl_bt_gatt_server_characteristic_status_flag_t)
              evt->data.evt_gatt_server_characteristic_status.status_flags) {
              bleData->indication_inFlight = false;
          }
          //track indication bool
      }
      break;

      //event indicates confirmation from the remote GATT client has not been received within 30 seconds after an indication was sent
    case sl_bt_evt_gatt_server_indication_timeout_id:

      LOG_INFO("server indication timeout\n\r");
      bleData->indication = false;
      break;

      //for clients


  }
}
