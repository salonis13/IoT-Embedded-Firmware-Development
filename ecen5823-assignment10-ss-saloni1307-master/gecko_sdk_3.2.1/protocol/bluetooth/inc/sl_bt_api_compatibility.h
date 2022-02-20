/***************************************************************************//**
 * @brief BT API source code compatibility for deprecated items
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

#ifndef SL_BT_API_COMPATIBILITY_H
#define SL_BT_API_COMPATIBILITY_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Deprecated and replaced by sl_bt_system_boot_mode_t.
 */
typedef enum
{
  system_boot_mode_normal   = 0x0,
  system_boot_mode_uart_dfu = 0x1,
  system_boot_mode_ota_dfu  = 0x2
} system_boot_mode_t;

/*
 * Deprecated and replaced by sl_bt_system_linklayer_config_key_t.
 */
typedef enum
{
  system_linklayer_config_key_halt                       = 0x1,
  system_linklayer_config_key_priority_range             = 0x2,
  system_linklayer_config_key_scan_channels              = 0x3,
  system_linklayer_config_key_set_flags                  = 0x4,
  system_linklayer_config_key_clr_flags                  = 0x5,
  system_linklayer_config_key_set_afh_interval           = 0x7,
  system_linklayer_config_key_set_priority_table         = 0x9,
  system_linklayer_config_key_set_rx_packet_filtering    = 0xa,
  system_linklayer_config_key_set_simultaneous_scanning  = 0xb,
  system_linklayer_config_key_set_channelmap_flags       = 0xc,
  system_linklayer_config_key_power_control_golden_range = 0x10
} system_linklayer_config_key_t;

/*
 * Deprecated and replaced by sl_bt_gap_address_type_t.
 */
typedef enum
{
  gap_public_address               = 0x0,
  gap_static_address               = 0x1,
  gap_random_resolvable_address    = 0x2,
  gap_random_nonresolvable_address = 0x3
} gap_address_type_t;



/*
 * Deprecated and replaced by sl_bt_advertiser_connectable_mode_t.
 */
typedef enum
{
  advertiser_non_connectable           = 0x0,
  advertiser_directed_connectable      = 0x1,
  advertiser_connectable_scannable     = 0x2,
  advertiser_scannable_non_connectable = 0x3,
  advertiser_connectable_non_scannable = 0x4
} advertiser_connectable_mode_t;

/*
 * Deprecated and replaced by sl_bt_advertiser_discoverable_mode_t.
 */
typedef enum
{
  advertiser_non_discoverable     = 0x0,
  advertiser_limited_discoverable = 0x1,
  advertiser_general_discoverable = 0x2,
  advertiser_broadcast            = 0x3,
  advertiser_user_data            = 0x4
} advertiser_discoverable_mode_t;

/*
 * Deprecated and replaced by sl_bt_advertiser_adv_address_type_t.
 */
typedef enum
{
  advertiser_identity_address = 0x0,
  advertiser_non_resolvable   = 0x1
} advertiser_adv_address_type_t;

/*
 * Deprecated and replaced by sl_bt_scanner_discover_mode_t.
 */
typedef enum
{
  scanner_discover_limited     = 0x0,
  scanner_discover_generic     = 0x1,
  scanner_discover_observation = 0x2
} scanner_discover_mode_t;

/*
 * Deprecated and replaced by sl_bt_sync_advertiser_clock_accuracy_t.
 */
typedef enum
{
  sync_clock_accuracy_500 = 0x1f4,
  sync_clock_accuracy_250 = 0xfa,
  sync_clock_accuracy_150 = 0x96,
  sync_clock_accuracy_100 = 0x64,
  sync_clock_accuracy_75  = 0x4b,
  sync_clock_accuracy_50  = 0x32,
  sync_clock_accuracy_30  = 0x1e,
  sync_clock_accuracy_20  = 0x14
} sync_advertiser_clock_accuracy_t;

/*
 * Deprecated and replaced by sl_bt_connection_security_t.
 */
typedef enum
{
  connection_mode1_level1 = 0x0,
  connection_mode1_level2 = 0x1,
  connection_mode1_level3 = 0x2,
  connection_mode1_level4 = 0x3
} connection_security_t;

/*
 * Deprecated and replaced by sl_bt_connection_power_reporting_mode_t.
 */
typedef enum
{
  connection_power_reporting_disable = 0x0,
  connection_power_reporting_enable  = 0x1
} connection_power_reporting_mode_t;

/*
 * Deprecated and replaced by sl_bt_connection_tx_power_flag_t.
 */
typedef enum
{
  connection_tx_power_at_minimum = 0x1,
  connection_tx_power_at_maximum = 0x2
} connection_tx_power_flag_t;

/*
 * Deprecated CONNECTION_* defines replaced by SL_BT_CONNECTION_*.
 */
#define CONNECTION_TX_POWER_UNMANAGED          0x7e      
#define CONNECTION_TX_POWER_UNAVAILABLE        0x7f      
#define CONNECTION_TX_POWER_CHANGE_UNAVAILABLE 0x7f      


/*
 * Deprecated and replaced by sl_bt_gatt_att_opcode_t.
 */
typedef enum
{
  gatt_read_by_type_request      = 0x8,
  gatt_read_by_type_response     = 0x9,
  gatt_read_request              = 0xa,
  gatt_read_response             = 0xb,
  gatt_read_blob_request         = 0xc,
  gatt_read_blob_response        = 0xd,
  gatt_read_multiple_request     = 0xe,
  gatt_read_multiple_response    = 0xf,
  gatt_write_request             = 0x12,
  gatt_write_response            = 0x13,
  gatt_write_command             = 0x52,
  gatt_prepare_write_request     = 0x16,
  gatt_prepare_write_response    = 0x17,
  gatt_execute_write_request     = 0x18,
  gatt_execute_write_response    = 0x19,
  gatt_handle_value_notification = 0x1b,
  gatt_handle_value_indication   = 0x1d
} gatt_att_opcode_t;

/*
 * Deprecated and replaced by sl_bt_gatt_client_config_flag_t.
 */
typedef enum
{
  gatt_disable      = 0x0,
  gatt_notification = 0x1,
  gatt_indication   = 0x2
} gatt_client_config_flag_t;

/*
 * Deprecated and replaced by sl_bt_gatt_execute_write_flag_t.
 */
typedef enum
{
  gatt_cancel = 0x0,
  gatt_commit = 0x1
} gatt_execute_write_flag_t;

/*
 * Deprecated and replaced by sl_bt_gattdb_service_type_t.
 */
typedef enum
{
  gattdb_primary_service   = 0x0,
  gattdb_secondary_service = 0x1
} gattdb_service_type_t;

/*
 * Deprecated and replaced by sl_bt_gattdb_value_type_t.
 */
typedef enum
{
  gattdb_fixed_length_value    = 0x1,
  gattdb_variable_length_value = 0x2,
  gattdb_user_managed_value    = 0x3
} gattdb_value_type_t;

/*
 * Deprecated GATTDB_* defines replaced by SL_BT_GATTDB_*.
 */
#define GATTDB_ADVERTISED_SERVICE 0x1       

#define GATTDB_ENCRYPTED_READ       0x1       
#define GATTDB_BONDED_READ          0x2       
#define GATTDB_AUTHENTICATED_READ   0x4       
#define GATTDB_ENCRYPTED_WRITE      0x8       
#define GATTDB_BONDED_WRITE         0x10      
#define GATTDB_AUTHENTICATED_WRITE  0x20      
#define GATTDB_ENCRYPTED_NOTIFY     0x40      
#define GATTDB_BONDED_NOTIFY        0x80      
#define GATTDB_AUTHENTICATED_NOTIFY 0x100     

#define GATTDB_NO_AUTO_CCCD         0x1       

#define GATTDB_CHARACTERISTIC_READ              0x2       
#define GATTDB_CHARACTERISTIC_WRITE_NO_RESPONSE 0x4       
#define GATTDB_CHARACTERISTIC_WRITE             0x8       
#define GATTDB_CHARACTERISTIC_NOTIFY            0x10      
#define GATTDB_CHARACTERISTIC_INDICATE          0x20      
#define GATTDB_CHARACTERISTIC_EXTENDED_PROPS    0x80      
#define GATTDB_CHARACTERISTIC_RELIABLE_WRITE    0x101     

#define GATTDB_DESCRIPTOR_READ                  0x1       
#define GATTDB_DESCRIPTOR_WRITE                 0x2       
#define GATTDB_DESCRIPTOR_LOCAL_ONLY            0x200     


/*
 * Deprecated and replaced by sl_bt_gatt_server_client_configuration_t.
 */
typedef enum
{
  gatt_server_disable                     = 0x0,
  gatt_server_notification                = 0x1,
  gatt_server_indication                  = 0x2,
  gatt_server_notification_and_indication = 0x3
} gatt_server_client_configuration_t;

/*
 * Deprecated and replaced by sl_bt_gatt_server_characteristic_status_flag_t.
 */
typedef enum
{
  gatt_server_client_config = 0x1,
  gatt_server_confirmation  = 0x2
} gatt_server_characteristic_status_flag_t;

/*
 * Deprecated NVM_* defines replaced by SL_BT_NVM_*.
 */
#define NVM_KEY_CTUNE 0x32      


/*
 * Deprecated and replaced by sl_bt_test_packet_type_t.
 */
typedef enum
{
  test_pkt_prbs9    = 0x0,
  test_pkt_11110000 = 0x1,
  test_pkt_10101010 = 0x2,
  test_pkt_11111111 = 0x4,
  test_pkt_00000000 = 0x5,
  test_pkt_00001111 = 0x6,
  test_pkt_01010101 = 0x7,
  test_pkt_pn9      = 0xfd,
  test_pkt_carrier  = 0xfe
} test_packet_type_t;

/*
 * Deprecated and replaced by sl_bt_test_phy_t.
 */
typedef enum
{
  test_phy_1m   = 0x1,
  test_phy_2m   = 0x2,
  test_phy_125k = 0x3,
  test_phy_500k = 0x4
} test_phy_t;

/*
 * Deprecated and replaced by sl_bt_sm_bonding_key_t.
 */
typedef enum
{
  sm_bonding_key_remote_ltk = 0x1,
  sm_bonding_key_local_ltk  = 0x2,
  sm_bonding_key_irk        = 0x3
} sm_bonding_key_t;

/*
 * Deprecated and replaced by sl_bt_sm_io_capability_t.
 */
typedef enum
{
  sm_io_capability_displayonly     = 0x0,
  sm_io_capability_displayyesno    = 0x1,
  sm_io_capability_keyboardonly    = 0x2,
  sm_io_capability_noinputnooutput = 0x3,
  sm_io_capability_keyboarddisplay = 0x4
} sm_io_capability_t;

/*
 * Deprecated and replaced by sl_bt_coex_option_t.
 */
typedef enum
{
  coex_option_enable        = 0x100,
  coex_option_tx_abort      = 0x400,
  coex_option_high_priority = 0x800
} coex_option_t;

/*
 * Deprecated and replaced by sl_bt_l2cap_coc_connection_result_t.
 */
typedef enum
{
  l2cap_connection_successful            = 0x0,
  l2cap_le_psm_not_supported             = 0x2,
  l2cap_no_resources_available           = 0x4,
  l2cap_insufficient_authentication      = 0x5,
  l2cap_insufficient_authorization       = 0x6,
  l2cap_insufficient_encryption_key_size = 0x7,
  l2cap_insufficient_encryption          = 0x8,
  l2cap_invalid_source_cid               = 0x9,
  l2cap_source_cid_already_allocated     = 0xa,
  l2cap_unacceptable_parameters          = 0xb
} l2cap_coc_connection_result_t;

/*
 * Deprecated and replaced by sl_bt_l2cap_command_reject_reason_t.
 */
typedef enum
{
  l2cap_command_not_understood = 0x0,
  l2cap_signaling_mtu_exceeded = 0x1,
  l2cap_invalid_cid_request    = 0x2
} l2cap_command_reject_reason_t;

/*
 * Deprecated and replaced by sl_bt_l2cap_command_code_t.
 */
typedef enum
{
  l2cap_disconnection_request = 0x6,
  l2cap_connection_request    = 0x14,
  l2cap_flow_control_credit   = 0x16
} l2cap_command_code_t;

/*
 * Deprecated MEMORY_PROFILER_* defines replaced by SL_BT_MEMORY_PROFILER_*.
 */
#define MEMORY_PROFILER_ERROR_TOO_MANY_TRACKERS         0x1       
#define MEMORY_PROFILER_ERROR_TOO_MANY_LIVE_ALLOCATIONS 0x2       

#define MEMORY_PROFILER_RESET_ERROR_FLAGS               0x1       
#define MEMORY_PROFILER_RESET_ALLOC_FAILURES            0x2       

/*
 * Deprecated and replaced by sl_bt_gap_phy_type_t.
 */
typedef enum
{
  gap_1m_phy    = 0x1,
  gap_2m_phy    = 0x2,
  gap_coded_phy = 0x4,
  gap_any_phys  = 0xff
} gap_phy_type_t;

/*
 * Deprecated and replaced by sl_bt_gap_phy_and_coding_type_t.
 */
typedef enum
{
  gap_1m_phy_uncoded = 0x1,
  gap_2m_phy_uncoded = 0x2,
  gap_coded_phy_125k = 0x4,
  gap_coded_phy_500k = 0x8
} gap_phy_and_coding_type_t;

/**
 * Deprecated and replaced by sl_bt_gap_phy_t.
 */
typedef enum
{
  sl_bt_gap_1m_phy    = 0x1,  /**< (0x1) 1M PHY */
  sl_bt_gap_2m_phy    = 0x2,  /**< (0x2) 2M PHY */
  sl_bt_gap_coded_phy = 0x4,  /**< (0x4) Coded PHY, 125k (S=8) or 500k (S=2) */
  sl_bt_gap_any_phys  = 0xff  /**< (0xff) Any PHYs the device supports */
} sl_bt_gap_phy_type_t;

/**
 * Deprecated and replaced by sl_bt_gap_phy_coding_t.
 */
typedef enum
{
  sl_bt_gap_1m_phy_uncoded = 0x1, /**< (0x1) 1M PHY */
  sl_bt_gap_2m_phy_uncoded = 0x2, /**< (0x2) 2M PHY */
  sl_bt_gap_coded_phy_125k = 0x4, /**< (0x4) 125k Coded PHY (S=8) */
  sl_bt_gap_coded_phy_500k = 0x8  /**< (0x8) 500k Coded PHY (S=2) */
} sl_bt_gap_phy_and_coding_type_t;

#ifdef __cplusplus
}
#endif

#endif