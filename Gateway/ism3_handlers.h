#ifndef ISM3_HANDLERS_
#define ISM3_HANDLERS_

// Comment out for console messages
//#define DEBUG_ISM3_HANDLERS

// Print to console
#ifdef DEBUG_ISM3_HANDLERS
#include <stdio.h>
#endif

#include <string.h> // memcpy
#include "ism3.h"

// EXPORTED VARIABLES



/**
 * POWER MODES:
 *
 * SM_TX -> for gateway
 * SM_RX_ACTIVE -> reacts to beacon data changes, receives async rx unicast frames, node is in SYNCHRONIZED state
 * SM_RX_LOW_POWER -> no group wakeup, node receives beacon data changes, no async rx
 * SM_RX_LOW_POWER_GROUP -> allows wakeup from gateway, node wakes up in SYNCHRONIZED state, async RX when woken up
 *
 * approx. 15s for node to notify lost sync
 * STM32 sometimes doesn't call handler on ISM state change
 */

/**
 * Do not modify headers unless they conform to definitions in ism3.h
 */

/**
 * @brief Unicast RX handler function
 * @param RX data from ism3
 * @param RX data length
 * @param source address
 * @param frame RSSI
 * @param frame LQI
 * Function is called when unicast frames are received
 */
void rx_unicast_handler(const uint8_t* data, uint8_t size, uint8_t source, int8_t rssi, uint8_t lqi);

/**
 * @brief Multicast RX handler function
 * @param RX data from ism3
 * @param RX data length
 * @param source address
 * @param frame countdown (frame position in multicast sequence)
 * @param frame RSSI
 * @param frame LQI
 * Function is called when multicast frames are received
 */
void rx_multicast_handler(const uint8_t* data, uint8_t size, uint8_t source, uint8_t countdown, int8_t rssi, uint8_t lqi);

/**
 * @brief Beacon sync data change handler
 * @param new beacon data
 * @param new beacon data size
 * Function is called on beacon sync data change
 */
void beacon_data_handler(const uint8_t* data, uint8_t size);

/**
 * @brief ISM state change handler
 * @param new ISM state
 * @param associated beacon id
 * Function is called on ISM state change
 * See implementation for state descriptions
 */
void state_handler(ism_state_t state, const uint8_t* gateway_id);

/**
 * @brief ISM stat RX handler
 * @param stat structure
 * Function is called when ISM sends its transmission statistics
 * Useful to debug a transmission or get usage statistics
 */
void stat_handler(ism_stat_t stat);


#endif // ISM3_HANDLERS_
