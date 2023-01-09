/**
 ******************************************************************************
 * @file    ism3.h
 * @author  nicolas.brunner@heig-vd.ch
 * @date    06-August-2018
 * @brief   Driver for the RM1S3
 ******************************************************************************
 * @copyright HEIG-VD
 *
 * License information
 *
 ******************************************************************************
 */

#ifndef ISM_H
#define ISM_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/

typedef enum {
    ISM_OFF,
    ISM_NOT_SYNCHRONIZED,
    ISM_SYNCHRONIZED,
    ISM_LOW_POWER_SYNC,
    ISM_TX_SYNC,
    ISM_VERSION_READY,
} ism_state_t;

typedef enum {
    SM_TX,
    SM_RX_ACTIVE,
    SM_RX_LOW_POWER,
    SM_RX_LOW_POWER_GROUP,
} ism_sync_mode_t;

typedef struct {
    uint32_t rxOk; // number of data frame correctly received
    uint32_t rxCrcError; // number of data frame received with CRC error

    uint32_t tx; // number of data frame transmission
    uint32_t txLbtFail; // number of data frame LBT failure
    uint32_t txAck; // number of data frame acknowledged
    uint32_t txNack; // number of data frame not acknowledged

    // in synchronized mode
    uint32_t syncRxOk; // number of successful sync frame reception
    uint32_t syncRxCrcError; // number of CRC error in sync frame reception
    uint32_t syncRxBadFrame; // number of bad sync frame received
    uint32_t syncRxTimeout; // number of sync frame reception timeout
    uint32_t syncRxLost; // number of synchronization lost
    int16_t syncMinDelta; // Minimum value for the delta of synchronization in tick
    int16_t syncMaxDelta; // Maximum value for the delta of synchronization in tick
    int32_t syncSumDelta; // Sum of all the delta of synchronization in tick

    // in low power synchronized mode
    uint32_t lpsyncRxOk; // number of successful sync frame reception
    uint32_t lpsyncRxCrcError; // number of CRC error in sync frame reception
    uint32_t lpsyncRxBadFrame; // number of bad sync frame received
    uint32_t lpsyncRxTimeout; // number of sync frame reception timeout
    uint32_t lpsyncRxLost; // number of synchronization lost
    int16_t lpsyncMinDelta; // Minimum value for the delta of synchronization in tick
    int16_t lpsyncMaxDelta; // Maximum value for the delta of synchronization in tick
    int32_t lpsyncSumDelta; // Sum of all the delta of synchronization in tick

    uint32_t syncTxOk; // number of sync frame transmission
    uint32_t syncTxLbtFail; // number of sync frame LBT failure

    uint32_t rxScanTime; // Time spend in reception during scan in second
    uint32_t rxTime; // total time spend in reception (excepted scan) in second
    uint32_t txTime; // total time spend in transmission in second

    // transmission at user level (nack only after all retry fail)
    uint32_t txUnicast; // Number of unicast demand (return no error)
    uint32_t txUnicastAck; // Number of acknowledged unicast
    uint32_t txUnicastNack; // Number of not acknowledged unicast
    uint32_t txMulticast; // Number of multicast demand (return no error)
    uint32_t txMulticastAttempt; // Number of multicast transmission attempt (txMulticast * (countdown + 1))

    uint32_t rxUnicastOk; // Number of correctly received unicast frame
    uint32_t rxMulticastOk; // Number of correctly received multicast frame
    uint32_t rxBadFrame; // Number of wrong frame type or size in a data slot
    uint32_t rxWrongBeaconId; // Number of received frame with the wrong beacon id
    uint32_t rxUnicastDuplicate; // Number of duplicate unicast frame received
    uint32_t rxUnicastWrongAddress; // Number of unicast frame with wrong destination address received
    uint32_t rxMulticastWrongGroup; // Number of multicast frame with wrong destination group received

    uint32_t scanOnPhase; // Number of phase with active scanning
    uint32_t scanLock; // Number of time a channel is locked because of RSSI over threshold
    uint32_t scanLockTimeout; // Number of time a locked channel timeout because of lack of pattern received
    uint32_t scanLockRssiTooLow; // Number of time a locked channel end because of a RSSI under threshold
    uint32_t scanRxCrcError; // Number of CRC error in scan frame reception
    uint32_t scanRxBadFrame; // Number of bad scan frame received
    uint32_t scanRefuseUnassociated; // Number of synchronization fail because of beacon refuse unassociated
    uint32_t scanSuccess; // Number of scan success
} ism_stat_t;

typedef void (*ism_unicast_function_t)(const uint8_t* data, uint8_t size, uint8_t source, int8_t rssi, uint8_t lqi);
typedef void (*ism_multicast_function_t)(const uint8_t* data, uint8_t size, uint8_t source, uint8_t countdown, int8_t rssi, uint8_t lqi);
typedef void (*ism_beacon_data_function_t)(const uint8_t* data, uint8_t size);
typedef void (*ism_state_function_t)(ism_state_t state, const uint8_t* gateway_id);
typedef void (*ism_stat_function_t)(ism_stat_t stat);

/* Exported preprocessor constants -------------------------------------------*/

#define ISM_TIMESLOT_DURATION 20
#define ISM_MAX_DATA_SIZE     239
#define ISM_INVALID_POWER     0xFF
#define ISM_MAX_POWER         52
#define ISM_MAX_POWER_DBM     30

/* Exported functions --------------------------------------------------------*/

/**
 * Initialize the ISM
 * @param rx_unicast_function the function called when unicast are received
 * @param rx_multicast_function the function called when multicast data are received
 * @param beacon_data_function the function called when beacon data are received
 * @param state_function the function called when the state change
 * @param stat_function the function called when stat are read
 */
void ism_init(
        ism_unicast_function_t rx_unicast_function,
        ism_multicast_function_t rx_multicast_function,
        ism_beacon_data_function_t beacon_data_function,
        ism_state_function_t state_function,
        ism_stat_function_t stat_function);

void ism_config(uint8_t address, uint32_t group, uint8_t power, uint8_t power_dbm, uint64_t associated_beacon_id);

void ism_get_config(uint8_t* address, uint32_t* group, uint8_t* power, uint8_t* power_dbm, uint64_t* associated_beacon_id);

/**
 * Set the physical layer parameters, they will be use only after a ism_disconnect()
 */
bool ism_set_phy(uint8_t phy, const uint8_t* channels);

void ism_disconnect(void);

void ism_set_sync_mode(ism_sync_mode_t mode);

void ism_tick(void);

bool ism_tx(uint8_t destination, const uint8_t* data, uint8_t size);

bool ism_is_tx_pending(void);

bool ism_is_ready(void);

uint8_t ism_get_max_data_size(void);

char* ism_get_firmware_version(void);

/**
 * Get the firmware version coded into an integer
 * version a.b.c => a << 16 + b << 8 + c
 * @return the firmware version coded into an integer
 */
uint32_t ism_get_firmware_version_value(void);

char* ism_get_hardware_version(void);

bool ism_update_firmware(const uint8_t* firmware, uint32_t size);

bool ism_request_stat(void);

bool ism_request_state(void);

uint32_t ism_get_uart_rx_counter(void);

uint8_t ism_get_channels_size(uint8_t phy);

bool send_command(const uint8_t* data);
#endif
