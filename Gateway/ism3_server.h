/**
 * @file ism3_server.h
 * @author marc.leemann@master.hes-so.ch
 * @brief Set of utilities for ISM module as a server operation.
 *
 * Wake/unwake functions allow waking up or putting to sleep a specific group.
 * Changing beacon data allows one-way broadcast data transmission.
 */

#ifndef ISM3_SERVER_H
#define ISM3_SERVER_H

#include <stdint.h>     // type definitions
#include <stdbool.h>    // bool type definition
#include <stdlib.h>     // malloc, free
#include <string.h>     // memcpy

#include "ism3.h"
#include "ism3_handlers.h"
#include "cm4_utils.h" // delay
#include "commands_RM1S3.h"

#define ISM_UID_SIZE    UID_SIZE ///< Unique identifier byte length

/**
 * @brief ISM server init function
 * @param power_ power setting for ISM
 * @param power_dbm_ power in dBm for correct ISM stats
 *
 * Initializes ISM module in SM_TX mode with address 0 and group 0xFFFFFFFF.
 * Function is blocking until ISM is ready. Minimum delay is 500 ms.
 * Master function for init, no need to call anything else.
 */
void ism_server_init(uint8_t power_, uint8_t power_dbm_);
/**
 * @brief ISM server power getter function
 * @return current power setting for ISM
 */
uint8_t ism_server_get_power();
/**
 * @brief ISM server power dbm getter function
 * @return current power dbm setting for ISM
 */
uint8_t ism_server_get_power_dbm();
/**
 * @brief ISM server set power function
 * @param newPower new power setting for ISM
 * @param newPower_dbm matching new dBm power for correct ISM stats
 *
 * Reconfigures ISM server power. Blocking like ism_server_init.
 */
void ism_server_set_power(uint8_t newPower, uint8_t newPower_dbm);
/**
 * @brief Get already awake groups
 * @return awake groups
 */
uint32_t ism_server_get_awake();
/**
 * @brief Wakeup argument low-power group
 * @param group group to wake up
 * @return true if command was sent to ISM module
 *
 * Will wake up argument group and keep already woken up groups awake.
 * To wake only argument group, call ism_server_unwake_groups first.
 */
bool ism_server_wakeup_group(uint32_t group);
/**
 * @brief Unwake argument low-power group
 * @param group group to unwake
 * @return true if command was sent to ISM module
 */
bool ism_server_unwake_group(uint32_t group);
/**
 * @brief Unwake all groups
 * @return true if command was sent to ISM module
 */
bool ism_server_unwake_groups();
/**
 * @brief Change beacon sync data
 * @param beaconData pointer to new beacon data
 * @param length new beacon data length
 * @return true if command was sent to ISM module
 *
 * Will wake all nodes on data change, so it is useful to set node parameters
 * remotely even if in an inappropriate power mode for unicast frame RX
 */
bool ism_server_change_beacon_data(uint8_t * beaconData, uint8_t length);

#endif /* ISM3_SERVER_H */
