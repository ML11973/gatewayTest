/**
 * @file ism3_handlers.h
 * @author marc.leemann@master.hes-so.ch
 * @brief Standard handlers for ISM3 driver callbacks
 *
 * See @subpage ISM3_powermodes and @subpage ISM3_states for complementary
 * explanations.
 *
 * These handlers are not mandatory, but why not use them now that they're here?
 *
 * Do not modify headers unless they conform to definitions in ism3.h
 */

#ifndef ISM3_HANDLERS_
#define ISM3_HANDLERS_


//#define DEBUG_ISM3_HANDLERS ///< Uncomment to signal handler entry in console

// Print to console
#ifdef DEBUG_ISM3_HANDLERS
#include <stdio.h>
#endif

#include <string.h> // memcpy
#include "ism3.h"

/**
 * @brief Unicast RX handler function
 * @param data pointer to RX data from ism3
 * @param size RX data length
 * @param source frame source address
 * @param rssi frame Received Signal Strength Indicator
 * @param lqi frame Link Quality Indicator
 *
 * Function is called when unicast frames are received
 */
void rx_unicast_handler(const uint8_t* data, uint8_t size, uint8_t source, int8_t rssi, uint8_t lqi);

/**
 * @brief Multicast RX handler function
 * @param data pointer to RX data from ism3
 * @param size RX data length
 * @param source frame source address
 * @param countdown frame countdown (frame position in multicast sequence)
 * @param rssi frame Received Signal Strength Indicator
 * @param lqi frame Link Quality Indicator
 *
 * Function is called when multicast frames are received
 */
void rx_multicast_handler(const uint8_t* data, uint8_t size, uint8_t source, uint8_t countdown, int8_t rssi, uint8_t lqi);

/**
 * @brief Beacon sync data change handler
 * @param data pointer to new beacon data
 * @param size new beacon data length
 *
 * Function is called on beacon sync data change
 */
void beacon_data_handler(const uint8_t* data, uint8_t size);

/**
 * @brief ISM state change handler
 * @param state new ISM state
 * @param gateway_id associated beacon id
 *
 * Function is called on ISM state change.
 * See implementation for state descriptions
 */
void state_handler(ism_state_t state, const uint8_t* gateway_id);

/**
 * @brief ISM stat RX handler
 * @param stat statistics structure
 *
 * Function is called when ISM sends its transmission statistics
 * Useful to debug a transmission or get usage statistics
 */
void stat_handler(ism_stat_t stat);


#endif // ISM3_HANDLERS_
