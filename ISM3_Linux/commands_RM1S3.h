/**
 ******************************************************************************
 * @file    commands_RM1S3.h
 * @author  nicolas.brunner@heig-vd.ch
 * @date    25-March-2013
 * @brief   List of the host commands
 ******************************************************************************
 * @copyright HEIG-VD
 *
 * License information
 *
 ******************************************************************************
 */

#ifndef COMMANDS_H
#define COMMANDS_H


// G means GENERATE_TDMA_RESTART (used only for SET commands)
// R means NEED_TDMA_RUNNING (used only for GET commands)

#define CMD_GET_FIRMWARE_VERSION          0x11

#define CMD_GET_HARDWARE_VERSION          0x13

#define CMD_GET_UNIQUE_DEVICE_ID          0x15

#define CMD_SET_DATA_SLOT_COUNT           0x20   // G
#define CMD_GET_DATA_SLOT_COUNT           0x21

#define CMD_GET_PROTOCOL_STATE            0x27

#define CMD_CLEAR_STAT                    0x2A
#define CMD_GET_STAT                      0x2B

#define CMD_SET_ACTIVE_CHANNELS           0x2C
#define CMD_GET_ACTIVE_CHANNELS           0x2D

#define CMD_SET_DATA_SLOT_RANGE_TYPE      0x3A

#define CMD_SET_DATA_SLOT_TYPE            0x3C
#define CMD_GET_DATA_SLOT_TYPE            0x3D

#define CMD_GET_LAST_RX_SYNC_INFO         0x31   // R

#define CMD_SET_PATTERN                   0x40   // G
#define CMD_GET_PATTERN                   0x41

#define CMD_SET_ENCRYPTION_KEY            0x42

#define CMD_SET_ADDRESS                   0x44
#define CMD_GET_ADDRESS                   0x45

#define CMD_SET_GROUP                     0x4C
#define CMD_GET_GROUP                     0x4D

#define CMD_SET_TX_RETRY_COUNT            0x5C
#define CMD_GET_TX_RETRY_COUNT            0x5D

#define CMD_SET_RF_POWER                  0x60
#define CMD_GET_RF_POWER                  0x61

#define CMD_SET_RF_PHY                    0x62   // G
#define CMD_GET_RF_PHY                    0x63

#define CMD_GET_PHYS_CHANNELS_PLAN_SIZE   0x65

#define CMD_SET_RF_POWER_DBM              0x66

#define CMD_SET_RADIO_MODE                0x70   // G
#define CMD_GET_RADIO_MODE                0x71

#define IND_TDMA_STATE_CHANGED            0x73

#define CMD_SET_EVENT_INDICATION          0x74
#define CMD_GET_EVENT_INDICATION          0x75

//////////  F A C T O R Y  //////////////////////////////

#define CMD_UNLOCK_FACTORY_CMD            0x90

#define CMD_GET_RF_TEST_BER               0x93

#define CMD_SET_RF_TEST_CHANNEL           0x94
#define CMD_GET_RF_TEST_CHANNEL           0x95

#define CMD_SET_RF_TEST_MODE              0x96   // G
#define CMD_GET_RF_TEST_MODE              0x97

#define CMD_GET_RSSI                      0x99

//////////  S Y N C  ////////////////////////////////////

#define CMD_SET_SYNC_MODE                 0xA0   // G
#define CMD_GET_SYNC_MODE                 0xA1

#define CMD_SET_SYNC_RX                   0xA2   // G
#define CMD_GET_SYNC_RX                   0xA3

#define CMD_SET_SYNC_RX_LOW_POWER         0xA4
#define CMD_GET_SYNC_RX_LOW_POWER         0xA5

#define CMD_SET_SYNC_TX_LOW_POWER         0xA6
#define CMD_GET_SYNC_TX_LOW_POWER         0xA7

#define CMD_SET_SYNC_BEACON_ID            0xA8
#define CMD_GET_SYNC_BEACON_ID            0xA9

#define CMD_SET_SYNC_USER_DATA            0xAA
#define CMD_GET_SYNC_USER_DATA            0xAB

#define CMD_GET_SYNC_TX_INTERVAL          0xAD

#define CMD_SET_ASSOCIATED_BEACON_ID      0xB0
#define CMD_GET_ASSOCIATED_BEACON_ID      0xB1

#define CMD_SET_ACCEPT_UNASSOCIATED       0xB2
#define CMD_GET_ACCEPT_UNASSOCIATED       0xB3

/////////////////////////////////////////////////////////

#define CMD_SET_HOST_BAUDRATE             0xC4
#define CMD_GET_HOST_BAUDRATE             0xC5

#define CMD_SET_TX_RETRY_RESTRICTION      0xC8
#define CMD_GET_TX_RETRY_RESTRICTION      0xC9

#define CMD_GET_RF_FRAME_MAX_SIZE         0xCB

#define CMD_SET_HOST_UART_SETTINGS        0xCC
#define CMD_GET_HOST_UART_SETTINGS        0xCD

#define CMD_SET_GPIO0_SIGNAL              0xE0
#define CMD_GET_GPIO0_SIGNAL              0xE1
#define CMD_SET_GPIO1_SIGNAL              0xE2
#define CMD_GET_GPIO1_SIGNAL              0xE3
#define CMD_SET_GPIO2_SIGNAL              0xE4
#define CMD_GET_GPIO2_SIGNAL              0xE5
#define CMD_SET_GPIO3_SIGNAL              0xE6
#define CMD_GET_GPIO3_SIGNAL              0xE7

#define CMD_SET_GPIO_OUT                  0xE8
#define CMD_GET_GPIO_IN                   0xE9
#define CMD_SET_CTS                       0xEA
#define CMD_GET_RTS                       0xEB

#define CMD_CLEAR_CLOCK_CHECK             0xEC
#define CMD_GET_CLOCK_CHECK               0xED

#define CMD_SEND_UNICAST                  0xD2   // R
#define IND_RECEIVED_UNICAST              0xD3

#define CMD_SEND_MULTICAST                0xD4   // R
#define IND_RECEIVED_MULTICAST            0xD5

#define IND_UPDATED_SYNC_DATA             0xD9


#define CMD_LOAD_DEFAULT_PARAMETERS       0xF6   // G

//////////  D E B U G  //////////////////////////////////

#define CMD_CLEAR_TIMESLOT_MAX_VALUES     0x3E
#define CMD_GET_TIMESLOT_MAX_VALUES       0x3F

#define CMD_GET_DEBUG_DATA                0xFF

/////////////////////////////////////////////////////////

#define IND_ERROR                         0x03

#endif
