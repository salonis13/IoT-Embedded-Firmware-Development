/***************************************************************************//**
 * @file
 * @brief Application Output LCD code
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/

// *****************************************************************************
// Dave Sluiter, 10-06-2021
// Modified from soc_btmesh_light example from Silicon Labs for
// ECEN 5823, IoT Embedded Firmware, University of Colorado, Boulder.
// Look for initials: DOS
// *****************************************************************************

#include <stdbool.h>
#include <stdio.h>
#include "em_common.h"
#include "sl_status.h"

#include "app.h"
#include "app_log.h"
#include "app_led.h"

#include "sl_bt_api.h"
#include "sl_btmesh_api.h"
#include "sl_btmesh_friend.h"
#include "sl_btmesh_ctl_server.h"
#include "sl_btmesh_lighting_server.h"
#include "sl_btmesh_provisioning_decorator.h"
#include "sl_btmesh_factory_reset.h"
#include "sl_btmesh_wstk_lcd.h"

// -----------------------------------------------------------------------------
// BT mesh Friend Node Callbacks

/*******************************************************************************
 * Called when the Friend Node establishes friendship with another node.
 *
 * @param[in] netkey_index Index of the network key used in friendship
 * @param[in] lpn_address Low Power Node address
 ******************************************************************************/
void sl_btmesh_friend_on_friendship_established(uint16_t netkey_index,
                                                uint16_t lpn_address)
{
  app_log("BT mesh Friendship established with LPN (netkey idx: %d, lpn addr: 0x%04x)\r\n",
          "(netkey idx: %d, lpn addr: 0x%04x)\r\n",
          netkey_index,
          lpn_address);
  sl_status_t status = sl_btmesh_LCD_write("FRIEND",
                                           BTMESH_WSTK_LCD_ROW_FRIEND);
  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
  (void)netkey_index;
  (void)lpn_address;
}

/*******************************************************************************
 * Called when the friendship that was successfully established with a Low Power
 * Node has been terminated.
 *
 * @param[in] netkey_index Index of the network key used in friendship
 * @param[in] lpn_address Low Power Node address
 * @param[in] reason Reason for friendship termination
 ******************************************************************************/
void sl_btmesh_friend_on_friendship_terminated(uint16_t netkey_index,
                                               uint16_t lpn_address,
                                               uint16_t reason)
{
  app_log("BT mesh Friendship terminated with LPN "
          "(netkey idx: %d, lpn addr: 0x%04x, reason: 0x%04x)\r\n",
          netkey_index,
          lpn_address,
          reason);
  sl_status_t status = sl_btmesh_LCD_write("NO LPN",
                                           BTMESH_WSTK_LCD_ROW_FRIEND);
  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
  (void)netkey_index;
  (void)lpn_address;
  (void)reason;
}

// -----------------------------------------------------------------------------
// BT mesh CTL Callbacks

/*******************************************************************************
 * Called when the UI shall be updated with the changed CTL Model state during
 * a transition. The rate of this callback can be controlled by changing the
 * CTL_SERVER_UI_UPDATE_PERIOD macro.
 *
 * @param[in] temperature Temperature of color.
 * @param[in] deltauv     Delta UV value.
 ******************************************************************************/
void sl_btmesh_ctl_on_ui_update(uint16_t temperature,
                                uint16_t deltauv)
{
  // Temporary buffer to format the LCD output text
  char tmp_str[LCD_ROW_LEN];
  char deltauv_str[8] = { 0 };

  sl_btmesh_ctl_server_snprint_deltauv(deltauv_str,
                                       sizeof(deltauv_str),
                                       deltauv);

  snprintf(tmp_str, LCD_ROW_LEN, "ColorTemp: %5uK", temperature);
  app_log("BT mesh CTL Color temperature: %5uK\r\n", temperature);
  //DOS sl_status_t status = sl_btmesh_LCD_write(tmp_str,
  //                                         BTMESH_WSTK_LCD_ROW_TEMPERATURE);
  //app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");

  snprintf(tmp_str, LCD_ROW_LEN, "Delta UV: %6s ", deltauv_str);
  app_log("BT mesh CTL Delta UV: %6s\r\n", deltauv_str);
  //DOS status = sl_btmesh_LCD_write(tmp_str, BTMESH_WSTK_LCD_ROW_DELTAUV);
  //app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
}

// -----------------------------------------------------------------------------
// BT mesh Lightning Server Callbacks

/*******************************************************************************
 * Called when the UI shall be updated with the changed state of
 * lightning server during a transition. The rate of this callback can be
 * controlled by changing the LIGHTING_SERVER_UI_UPDATE_PERIOD macro.
 *
 * @param[in] lightness_level lightness level (0x0001 - FFFE)
 ******************************************************************************/
// DOS
// Calling sequence is:
//
// init_models(void)
//   generic_server_register_handler(MESH_GENERIC_ON_OFF_SERVER_MODEL_ID,
//                                   BTMESH_LIGHTING_SERVER_MAIN,
//                                   onoff_request, // <<== register the callback
//                                   onoff_change,
//                                   onoff_recall);
//
//   onoff_request()
//     sl_btmesh_lighting_set_level()
//       sl_btmesh_lighting_server_on_ui_update()
//
void sl_btmesh_lighting_server_on_ui_update(uint16_t lightness_level)
{
  // Temporary buffer to format the LCD output text
  char tmp_str[LCD_ROW_LEN];
  uint16_t lightness_percent = (lightness_level * 100 + 99) / 65535;

  // DOS
  // Students:
  // This is where we'll get lightness level changes to update the LCD display
  // in response to a receiving a Generic On/Off message from a Client (Publisher).
  //
  // lightness_percent ranges from 0 to 100.
  //
  // lightness_percent = 0   -> Full Off
  // lightness_percent = 100 -> Full On
  //
  // Clamp to full-off or full-on values
  if (lightness_percent < 50) {
      lightness_percent = 0;
  } else {
      lightness_percent = 100;
  }

  app_log("BT mesh Lightness: %5u%% (clamped)\r\n", lightness_percent); // DOS

  //DOS snprintf(tmp_str, LCD_ROW_LEN, "Lightness: %5u%%", lightness_percent);
  //
  // Students: Add code to display on row BTMESH_WSTK_LCD_ROW_LIGHTNESS
  //           of the LCD:
  //           "Light is Off" when lightness_percent == 0, and
  //           "Light is On"  when lightness_percent == 100
  //           AND
  //           when lightness_percent ==   0 turn off LED0 and LED1, and
  //           when lightness_percent == 100 turn on  LED0 and LED1
  //              using calls to app_led_set_level() from app_led.c

  if(lightness_percent == 0) {
      snprintf(tmp_str, LCD_ROW_LEN, "Light is Off");
      app_led_set_level(0);
  }
  else if(lightness_percent == 100) {
      snprintf(tmp_str, LCD_ROW_LEN, "Light is On");
      app_led_set_level(65535);
  }




  sl_status_t status = sl_btmesh_LCD_write(tmp_str,
                                           BTMESH_WSTK_LCD_ROW_LIGHTNESS);

  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
}

// -----------------------------------------------------------------------------
// Provisioning Decorator Callbacks

/*******************************************************************************
 * Called at node initialization time to provide provisioning information
 *
 * @param[in] provisioned  true: provisioned, false: unprovisioned
 * @param[in] address      Unicast address of the primary element of the node.
                           Ignored if unprovisioned.
 * @param[in] iv_index     IV index for the first network of the node
                           Ignored if unprovisioned.
 ******************************************************************************/
void sl_btmesh_on_provision_init_status(bool provisioned,
                                        uint16_t address,
                                        uint32_t iv_index)
{
  if (provisioned) {
    app_show_btmesh_node_provisioned(address, iv_index);
  } else {
    app_log("BT mesh node is unprovisioned, "
            "started unprovisioned beaconing...\r\n");
    sl_status_t status = sl_btmesh_LCD_write("unprovisioned",
                                             BTMESH_WSTK_LCD_ROW_STATUS);
    app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
  }
}

/*******************************************************************************
 * Called when the Provisioning starts
 *
 * @param[in] result  Result code. 0: success, non-zero: error
 ******************************************************************************/
void app_show_btmesh_node_provisioning_started(uint16_t result)
{
  app_log("BT mesh node provisioning is started (result: 0x%04x)\r\n",
          result);
  sl_status_t status = sl_btmesh_LCD_write("provisioning...",
                                           BTMESH_WSTK_LCD_ROW_STATUS);
  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
  (void)result;
}

/*******************************************************************************
 * Called when the Provisioning finishes successfully
 *
 * @param[in] address      Unicast address of the primary element of the node.
                           Ignored if unprovisioned.
 * @param[in] iv_index     IV index for the first network of the node
                           Ignored if unprovisioned.
 ******************************************************************************/
void app_show_btmesh_node_provisioned(uint16_t address,
                                      uint32_t iv_index)
{
  app_log("BT mesh node is provisioned (address: 0x%04x, iv_index: 0x%x)\r\n",
          address,
          iv_index);
  sl_status_t status = sl_btmesh_LCD_write("provisioned",
                                           BTMESH_WSTK_LCD_ROW_STATUS);
  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
  (void)address;
  (void)iv_index;
}

/*******************************************************************************
 * Called when the Provisioning fails
 *
 * @param[in] result  Result code. 0: success, non-zero: error
 ******************************************************************************/
void sl_btmesh_on_node_provisioning_failed(uint16_t result)
{
  app_log("BT mesh node provisioning failed (result: 0x%04x)\r\n", result);
  sl_status_t status = sl_btmesh_LCD_write("prov failed...",
                                           BTMESH_WSTK_LCD_ROW_STATUS);
  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
  (void)result;
}

// -----------------------------------------------------------------------------
// Factory Reset Callback

/*******************************************************************************
 * Shows the node reset information
 ******************************************************************************/
void app_show_btmesh_node_reset(void)
{
  app_log("Node reset\r\n");
  sl_status_t status = sl_btmesh_LCD_write("Node reset",
                                           BTMESH_WSTK_LCD_ROW_STATUS);
  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
}

/*******************************************************************************
 * Called when full reset is established, before system reset
 ******************************************************************************/
void sl_btmesh_factory_reset_on_full_reset(void)
{
  app_log("Factory reset\r\n");
  sl_status_t status = sl_btmesh_LCD_write("Factory reset",
                                           BTMESH_WSTK_LCD_ROW_STATUS);
  app_log_status_level_f(APP_LOG_LEVEL_ERROR, status, "LCD write failed");
}
