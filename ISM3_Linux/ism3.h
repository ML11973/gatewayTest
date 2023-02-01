/**
 ******************************************************************************
 * @file    ism3.h
 * @author  nicolas.brunner@heig-vd.ch, marc.leemann@master.hes-so.ch
 * @date    06-August-2018
 * @brief   Driver for the RM1S3
 ******************************************************************************
 * @copyright HEIG-VD
 *
 * Original STM32 driver by Nicolas Brunner.
 * Linux port and documentation by Marc Leemann.
 *
 ******************************************************************************
 */

/**
 * @page ISM3_manual Using the ISM3 module
 *
 * ## Overview
 *
 * The ISM3 radio module is a serial-accessible radio module for LPWAN
 * applications. It operates in the ISM 868 or 915 MHz band.
 *
 * Note: ISM stands for <a href="https://en.wikipedia.org/wiki/ISM_radio_band">Industrial-Scientific-Medical</a>,
 * which refers to the common name given to its operating band.
 * The ISM 868 band is an unlicensed (meaning anyone can use it as long as they
 * follow some common rules) band with center frequency at 868 MHz.
 * It is used for IoT applications in Europe. Its US counterpart is at 915 MHz.
 * There are other ISM bands, such as the 2.4 and 5 GHz bands used by Wi-Fi.
 *
 * ## Usage
 *
 * Initialize handler functions (@ref ism_init with @ref ism3_handlers.h as
 * arguments), configure module and set sync mode.
 *
 * @ref ism_tick regularly to update serial RX/TX buffers.
 * Without ticking, data may neither be transmitted nor received.
 *
 * Transmit frames with @ref ism_tx.
 *
 * Receive frames with configured handlers (see @ref ism_init).
 *
 * ## Configuration
 * See @subpage ISM3_config
 *
 * ## Power modes
 * See @subpage ISM3_syncmodes
 *
 * ## States
 * See @subpage ISM3_states
 *
 * ## Troubleshooting
 *
 * See @subpage ISM3_troubleshooting
 */
/**
 * @page ISM3_config Configuring the ISM3 module
 *
 * ## Where is the config?
 *
 * Configuration settings are scattered across:
 * - @ref ism_config
 * - @ref ism_set_sync_mode
 * - the @ref configuration_commands array
 * - the #BAUDRATE macro
 *
 * ## How do I change it?
 *
 * Use @ref ism_config to set address, group, power and beacon ID to sync to.
 *
 * Use @ref ism_set_sync_mode to set the desired sync mode.
 * See @ref ISM3_syncmodes for help on which one does what.
 *
 * Modify the configuration_commands array's contents to match your liking.
 * Refer to document
 * <a href="RM1S3 Host Commands.pdf">RM1S3 Host Commands</a> for settings information.
 * You can simply look up the command code and the command reference should
 * tell you what the parameter sets.
 *
 * **Attention**: Changing the baudrate in @ref configuration_commands is not
 * enough on its own. Baudrate must also be changed in #BAUDRATE macro for ISM3
 * baudrate to match host baudrate.
 *
 * ## Default gateway configuration
 *
 * A default configuration is available with @ref ism_server_init.
 * This configuration is called from wpanManager constructor.
 * See implementation for details.
 *
 * ### ism_config
 *
 * Call with following parameters:
 *
 * Name                 | Value
 * ---------------------|-----------
 * address_             | 0
 * group_               | 0xFFFFFFFF
 * power_               | 0x10
 * power_dbm_           | 0x12
 * associated_beacon_id | any (default: 0xD322FE7D02D3D117)
 *
 * ### ism_set_sync_mode
 *
 * Set as @ref SM_TX. See @ref ISM3_syncmodes.
 *
 * ### configuration_commands array
 *
 * @code{c}
static commands_t configuration_commands = {
    {0x04, CMD_SET_HOST_BAUDRATE,     0x00, 0x4B, 0x00}, // 19200 in hex notation
    {0x01, CMD_GET_FIRMWARE_VERSION},
    {0x01, CMD_GET_HARDWARE_VERSION},
	{0x01, CMD_GET_UNIQUE_DEVICE_ID},
    {0x05, CMD_SET_PATTERN,           0x19, 0x17, 0x10, 0x25},
    {0x01, CMD_GET_PHYS_CHANNELS_PLAN_SIZE},
    {0x02, CMD_SET_TX_RETRY_COUNT,    0x02}, // Tx retry count = 2
    {0x02, CMD_SET_GPIO0_SIGNAL,      0x01}, // GPIO0 = SIG_SYNC_OUT
    {0x02, CMD_SET_GPIO1_SIGNAL,      0x02}, // GPIO1 = SIG_TIMESLOT_OUT
    {0x02, CMD_SET_GPIO2_SIGNAL,      0x04}, // GPIO2 = SIG_TX_PENDING_OUT

#ifdef NDEBUG
    // interval = 1, missMax = 100, initialTrack = 402ms, trackOn = 202ms, trackOff = 400s
    {0x0A, CMD_SET_SYNC_RX,           1, 100, 0x01, 0x92, 0x00, 0xCA, 0x06, 0x1A, 0x80},
    // {0x0A, CMD_SET_SYNC_RX, 1, 100, 0x01, 0x92, 0x00, 0xCA, 0x00, 0xEA, 0x60}, // 60s
#else
    // interval = 1, missMax = 100, initialTrack = 402ms, trackOn = 202ms, trackOff = 5s
    {0x0A, CMD_SET_SYNC_RX,           1, 100, 0x01, 0x92, 0x00, 0xCA, 0x00, 0x13, 0x88},
#endif

    {0x02, CMD_SET_EVENT_INDICATION,  1}, // Enable IND_TDMA_STATE_CHANGED

    {0x02, CMD_SET_RADIO_MODE,        0x00}, // Stop TDMA
    {0x02, CMD_SET_RF_PHY,            0x00}, // use phy as parameter
    {0x02, CMD_SET_ACTIVE_CHANNELS,   0x00}, // use channels as parameter
    // SERVER CONFIG
    {0x02, CMD_SET_DATA_SLOT_COUNT, 0xF}, // 15 data slots

    // Following parameters can change during execution, the numbers of commands should be set in NUMBER_OF_RECONFIGURATION_CMD
    {0x02, CMD_SET_SYNC_MODE,         0x00}, // use sync_mode as parameter
    {0x06, CMD_SET_SYNC_RX_LOW_POWER, 0x00, 0x00, 0x00, 0x00, 25}, // use group as parameter, sync interval = 25
    {0x02, CMD_SET_RF_POWER,          0x00}, // use power as parameter
    {0x02, CMD_SET_ADDRESS,           0x00}, // use address as parameter
    {0x05, CMD_SET_GROUP,             0x00, 0x00, 0x00, 0x00}, // use group as parameter
    {0x09, CMD_SET_ASSOCIATED_BEACON_ID, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // use associated_beacon_id as parameter
    {0x04, CMD_SET_DATA_SLOT_RANGE_TYPE, 0, 254, 0x01}, // enable RX on all data slots
    {0x03, CMD_SET_DATA_SLOT_TYPE,    0x00, 0x00},
    {0x02, CMD_SET_RADIO_MODE,        0x01}, // Start TDMA
    {0x00}
}; ///< Configuration commands
 *
 * @endcode

 * ### BAUDRATE macro
 *
 * This macro defines linux hardware baudrate.
 * It should match baudrate set in configuration_commands.
 * Default value is 19200.
 * It should not be set as a random value since CM4 hardware only supports a
 * few fixed baudrates. See linux_uart_init source in buffered_uart.c and
 * <a href="https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/#baud-rate"> this blog post</a> for
 * additional information about UNIX compliant baudrates.
 *
 * ## What can I change?
 *
 * You can change host baudrate, pattern, TX retry count, GPIO signals, Sync RX,
 * RF Phy and channels, data slot count, sync mode, sync rx interval, power,
 * address and group.
 *
 * To keep gateway functionality, do not change sync mode, address and group.
 *
 */
/**
 * @page ISM3_syncmodes ISM3 sync modes
 *
 * The ISM module used in has several sync modes for the user to choose from.
 * The following descriptions should help the user choose the relevant mode for
 * their application.
 *
 * ### ISM_TX
 *
 * Module is in gateway mode. It will be the master of its WPAN
 * (Wireless Personal Area Network).
 *
 * ### SM_RX_ACTIVE
 *
 * Module is in RX mode. It will remain in SYNCHRONIZED state all the time, and
 * receive unicast and multicast frames. The module will not enter power saving
 * mode.
 *
 * ### SM_RX_LOW_POWER
 *
 * Module is in low power mode. It will not react to group wakeups and remain
 * asleep. The only way for the module to receive data is to get it from a
 * beacon data change.
 *
 * ### SM_RX_LOW_POWER_GROUP
 *
 * Module is in low power mode and will react to group wakeups.
 * When the gateway wakes one of the groups the module belongs to, it will enter
 * state ISM_SYNCHRONIZED.
 * When the node is synchronized, it will receive unicast and multicast frames.
 */
/**
 * @page ISM3_states ISM3 states
 *
 * General information: It takes approximately 15 seconds for a node to lose
 * sync with the current configuration in ism3.c.
 * This is because the node tries several times to reach the gateway until it
 * determines it has lost sync.
 *
 * ### ISM_OFF
 *
 * Module is uninitialized.
 *
 * ### ISM_NOT_SYNCHRONIZED
 *
 * Module is waiting for sync. It is not connected to a gateway.
 *
 * ### ISM_SYNCHRONIZED
 *
 * Module is synchronized to a gateway.
 * Gets in this state on wakeup from gateway if it is in SM_RX_LOW_POWER_GROUP
 * power mode.
 * Stays in this state for as long as it is connected in SM_RX_ACTIVE power mode.
 *
 * ### ISM_LOW_POWER_SYNC
 *
 * Module is synced to a gateway in SM_LOW_POWER or SM_LOW_POWER_GROUP power
 * mode, but not woken up. Cannot be woken up in SM_LOW_POWER.
 *
 * ### ISM_TX_SYNC
 *
 * Module is the gateway of its WPAN.
 *
 * ### ISM_VERSION_READY
 *
 * Module is getting initialized and has not received a power mode.
 */
/**
 * @page ISM3_troubleshooting ISM3 Troubleshooting
 *
 * Things don't always happen as planned and that is a normal part of
 * development.
 * There are a few standard steps one can take to identify problems.
 *
 * ## Documents
 *
 * <a href="RM1S3 Host Commands.pdf">ISM3 documentation</a>
 *
 * <a href="schema_cm4.PDF">Pi CM4 board schematic</a>
 *
 * <a href="RFShield_docs.PDF">Shield schematics and layout</a>
 *
 * ## Jumper setup
 *
 * There are a few jumpers to choose different configurations on the radio
 * shield:
 * - X11: power source. Left: power from NUCLEO connector.
 * Right: power from USB-C connector.
 * - X17,X18,X22,X23: UART source. Left: USB-C UART.
 * Right: UART from NUCLEO connector.
 * - X1,X5: Power supply toggles. Left jumper closes power supply circuit.
 * Right jumper closes power supply indicator LED circuit.
 * - X24: Integrated STM32 chip BOOT0 pin. Boot partition selection.
 * Do not bridge unless you know what you are doing.
 * Contact nicolas.brunner@heig-vd for information.
 *
 * Reference configuration for use with CM4 is:
 *
 * Name | Side
 * -----|-----
 * X1   | On (vertical)
 * X5   | On (vertical)
 * X11  | Left
 * X17  | Right
 * X18  | Right
 * X22  | Right
 * X23  | Right
 * X24  | No jumper
 *
 * This configuration is shown on radio module picture (@ref hardware_setup).
 *
 * ## Module power supply
 *
 * On the radio shield, check LEDs VCC1 and VCC_PA are lit up when jumpers X1, X5
 * are set.
 * If not lit up, check jumper X11.
 * Default setting is left for power coming from CM4 board.
 *
 * ## UART troubleshooting
 *
 * ### Check hardware connections
 * Check jumpers X17, X18, X22, X23 are correctly set.
 *
 * UART Jumper definitions:
 *
 * Name | Signal | Description
 * -----|--------|----------
 * X17  | CTS    | Clear to send
 * X18  | TX     | Transmit
 * X22  | RTS    | Request to send
 * X23  | RX     | Receive
 *
 * <a href="https://www.silabs.com/documents/public/application-notes/an0059.0-uart-flow-control.pdf">AN0059.0 from Silicon Labs</a>
 * provides a good overview of these signals and their waveforms.
 *
 * ### Check Linux serial
 *
 * The CM4 ISM3 driver uses serial port /dev/ttyAMA1 defined in #UART_DEV.
 * Check that this device links to UART4 on CM4.
 * Check that correct device tree overlay is used: on CM4 file system, check
 * that line dtoverlay=uart4 is present in /boot/config.txt.
 * This binds UART4 to a /dev serial link in the Linux operating system.
 * If your application uses other UARTs, check which /dev/ttyAMAX maps to which
 * serial hardware device.
 *
 * A serial test loopback program allows easy testing from a host PC with a
 * USB to serial adapter connected on the relevant port.
 * @todo link relevant project.
 *
 * ### Check UART signals
 *
 * Check that UART frames are sent on the UART.
 * Use a serial test program or <a href="https://unix.stackexchange.com/questions/117037/how-to-send-data-to-a-serial-port-and-see-any-answer">write directly</a> through command line with:
 *
 * @code{.sh}
 * echo -ne '\033[2J' > /dev/ttyAMA1
 * @endcode
 *
 * Read with
 *
 * @code{.sh}
 * cat -v < /dev/ttyAMA1
 * @endcode
 *
 * If no results are obtained, using an oscilloscope or logic analyzer will
 * allow you to check for UART frames manually.
 * If shield sends an answer, you are probably on the right track!
 * See error code definitions in <a href="RM1S3 Host Commands.pdf">ISM3 doc</a>
 * and try to solve the problem.
 *
 * If shield does not send an answer, baudrate configuration may be wrong.
 * Check config using @ref ISM3_config as reference.
 *
 * ## Shield configuration
 *
 * In gateway mode, check that LEDs IO1 and IO2 light up (default configuration).
 * If they don't, try to restart the program after resetting ISM3 module by
 * pressing the reset button on the left side.
 *
 * Default configuration LED signals:
 *
 * LED | Signal                 | Description
 * ----|------------------------|----
 * 1   | SIG_SYNC_OUT           | Beacon sent
 * 2   | SIG_TIMESLOT_OUT       | TX/RX timeslot
 * 3   | SIG_TX_PENDING_OUT     | Pending transmission
 *
 *
 * ## Radio communication
 *
 * If configuration LEDs light up, radio link may be defective.
 * Check R15-R16 position matches desired antenna setup.
 * Check impedance matching on C21, C23, C24.
 * If using on-shield antenna, check C25 and C26.
 *
 */

#ifndef ISM_H
#define ISM_H

/* Includes ------------------------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

/* Exported types ------------------------------------------------------------*/
/**
 * @brief ISM TDMA state. See @ref ISM3_states
 */
typedef enum {
    ISM_OFF,
    ISM_NOT_SYNCHRONIZED,
    ISM_SYNCHRONIZED,
    ISM_LOW_POWER_SYNC,
    ISM_TX_SYNC,
    ISM_VERSION_READY,
} ism_state_t;
/**
 * @brief ISM sync mode. See @ref ISM3_syncmodes
 */
typedef enum {
    SM_TX,
    SM_RX_ACTIVE,
    SM_RX_LOW_POWER,
    SM_RX_LOW_POWER_GROUP,
} ism_sync_mode_t;
/**
 * @brief Easy access to ISM statistics. Request with @ref ism_request_stat
 */
typedef struct {
    uint32_t rxOk; ///< number of data frame correctly received
    uint32_t rxCrcError; ///< number of data frame received with CRC error

    uint32_t tx; ///< number of data frame transmission
    uint32_t txLbtFail; ///< number of data frame LBT failure
    uint32_t txAck; ///< number of data frame acknowledged
    uint32_t txNack; ///< number of data frame not acknowledged

    // in synchronized mode
    uint32_t syncRxOk; ///< number of successful sync frame reception
    uint32_t syncRxCrcError; ///< number of CRC error in sync frame reception
    uint32_t syncRxBadFrame; ///< number of bad sync frame received
    uint32_t syncRxTimeout; ///< number of sync frame reception timeout
    uint32_t syncRxLost; ///< number of synchronization lost
    int16_t syncMinDelta; ///< Minimum value for the delta of synchronization in tick
    int16_t syncMaxDelta; ///< Maximum value for the delta of synchronization in tick
    int32_t syncSumDelta; ///< Sum of all the delta of synchronization in tick

    // in low power synchronized mode
    uint32_t lpsyncRxOk; ///< number of successful sync frame reception
    uint32_t lpsyncRxCrcError; ///< number of CRC error in sync frame reception
    uint32_t lpsyncRxBadFrame; ///< number of bad sync frame received
    uint32_t lpsyncRxTimeout; ///< number of sync frame reception timeout
    uint32_t lpsyncRxLost; ///< number of synchronization lost
    int16_t lpsyncMinDelta; ///< Minimum value for the delta of synchronization in tick
    int16_t lpsyncMaxDelta; ///< Maximum value for the delta of synchronization in tick
    int32_t lpsyncSumDelta; ///< Sum of all the delta of synchronization in tick

    uint32_t syncTxOk; ///< number of sync frame transmission
    uint32_t syncTxLbtFail; ///< number of sync frame LBT failure

    uint32_t rxScanTime; ///< Time spend in reception during scan in second
    uint32_t rxTime; ///< total time spend in reception (excepted scan) in second
    uint32_t txTime; ///< total time spend in transmission in second

    // transmission at user level (nack only after all retry fail)
    uint32_t txUnicast; ///< Number of unicast demand (return no error)
    uint32_t txUnicastAck; ///< Number of acknowledged unicast
    uint32_t txUnicastNack; ///< Number of not acknowledged unicast
    uint32_t txMulticast; ///< Number of multicast demand (return no error)
    uint32_t txMulticastAttempt; ///< Number of multicast transmission attempt (txMulticast * (countdown + 1))

    uint32_t rxUnicastOk; ///< Number of correctly received unicast frame
    uint32_t rxMulticastOk; ///< Number of correctly received multicast frame
    uint32_t rxBadFrame; ///< Number of wrong frame type or size in a data slot
    uint32_t rxWrongBeaconId; ///< Number of received frame with the wrong beacon id
    uint32_t rxUnicastDuplicate; ///< Number of duplicate unicast frame received
    uint32_t rxUnicastWrongAddress; ///< Number of unicast frame with wrong destination address received
    uint32_t rxMulticastWrongGroup; ///< Number of multicast frame with wrong destination group received

    uint32_t scanOnPhase; ///< Number of phase with active scanning
    uint32_t scanLock; ///< Number of time a channel is locked because of RSSI over threshold
    uint32_t scanLockTimeout; ///< Number of time a locked channel timeout because of lack of pattern received
    uint32_t scanLockRssiTooLow; ///< Number of time a locked channel end because of a RSSI under threshold
    uint32_t scanRxCrcError; ///< Number of CRC error in scan frame reception
    uint32_t scanRxBadFrame; ///< Number of bad scan frame received
    uint32_t scanRefuseUnassociated; ///< Number of synchronization fail because of beacon refuse unassociated
    uint32_t scanSuccess; ///< Number of scan success
} ism_stat_t;

typedef void (*ism_unicast_function_t)(const uint8_t* data, uint8_t size, uint8_t source, int8_t rssi, uint8_t lqi); ///< type of unicast RX handler
typedef void (*ism_multicast_function_t)(const uint8_t* data, uint8_t size, uint8_t source, uint8_t countdown, int8_t rssi, uint8_t lqi); ///< type of multicast RX handler
typedef void (*ism_beacon_data_function_t)(const uint8_t* data, uint8_t size); ///< type of beacon data change handler
typedef void (*ism_state_function_t)(ism_state_t state, const uint8_t* gateway_id); ///< type of state change handler
typedef void (*ism_stat_function_t)(ism_stat_t stat); ///< type of statistics RX handler

/* Exported preprocessor constants -------------------------------------------*/

#define ISM_TIMESLOT_DURATION 20   ///< Timeslot duration
#define ISM_MAX_DATA_SIZE     239  ///< Max frame size for TX
#define ISM_INVALID_POWER     0xFF ///< Power error code
#define ISM_MAX_POWER         52   ///< Max power
#define ISM_MAX_POWER_DBM     30   ///< Max power in dBm
#define UID_SIZE              12   ///< UID size in bytes

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Initialize ISM module
 * @param rx_unicast_function the function called when unicast are received
 * @param rx_multicast_function the function called when multicast data are received
 * @param beacon_data_function the function called when beacon data are received
 * @param state_function the function called when the state change
 * @param stat_function the function called when stat are read
 *
 * Initialize serial communication hardware and handler functions
 */
void ism_init(
        ism_unicast_function_t rx_unicast_function,
        ism_multicast_function_t rx_multicast_function,
        ism_beacon_data_function_t beacon_data_function,
        ism_state_function_t state_function,
        ism_stat_function_t stat_function);
/**
 * @brief Configure the ISM
 * @param address module address
 * @param group module group
 * @param power desired power setting
 * @param power_dbm matching power in dBm
 * @param associated_beacon_id beacon ID to synchronize to
 */
void ism_config(uint8_t address, uint32_t group, uint8_t power, uint8_t power_dbm, uint64_t associated_beacon_id);
/**
 * @brief Get current ISM config
 * @param address fill this memory zone with module address
 * @param group fill this memory zone with module group
 * @param power fill this memory zone with desired power setting
 * @param power_dbm fill this memory zone with matching power in dBm
 * @param associated_beacon_id fill this memory zone with beacon ID to synchronize to
 */
void ism_get_config(uint8_t* address, uint32_t* group, uint8_t* power, uint8_t* power_dbm, uint64_t* associated_beacon_id);
/**
 * @brief Get module unique identifier
 * @param uid_ pointer to array to store uid into
 * @param uid_size_ size of array. Must be > #UID_SIZE
 */
void ism_get_uid(uint8_t* uid_, uint8_t uid_size_);

/**
 * Set the physical layer parameters, they will be use only after a ism_disconnect()
 */
bool ism_set_phy(uint8_t phy, const uint8_t* channels);
/**
 * @brief Stop ISM module and restart
 */
void ism_disconnect(void);
/**
 * @brief Set ISM sync mode
 * @param mode new sync mode
 */
void ism_set_sync_mode(ism_sync_mode_t mode);
/**
 * @brief Tick. Poll for new RX data or need for module reconfiguration
 */
void ism_tick(void);
/**
 * @brief TX unicast frame
 * @param destination destination address
 * @param data pointer to data to transmit
 * @param size size of data to transmit (<#ISM_MAX_DATA_SIZE)
 */
bool ism_tx(uint8_t destination, const uint8_t* data, uint8_t size);
/**
 * @brief TX multicast frame
 * @param group groups to broadcast to
 * @param number number of frames to send
 * @param data pointer to data to transmit
 * @param size size of data to transmit (<#ISM_MAX_DATA_SIZE)
 */
bool ism_broadcast(uint32_t group, uint8_t number, const uint8_t* data, uint8_t size);

bool ism_is_tx_pending(void);

bool ism_is_ready(void);

uint8_t ism_get_max_data_size(void);

char* ism_get_firmware_version(void);

/**
 * @brief Get the firmware version coded into an integer.
 * Version a.b.c => a << 16 + b << 8 + c
 * @return the firmware version coded into an integer
 */
uint32_t ism_get_firmware_version_value(void);

char* ism_get_hardware_version(void);

bool ism_update_firmware(const uint8_t* firmware, uint32_t size);
/**
 * @brief Request communication statistics from ISM module
 */
bool ism_request_stat(void);
/**
 * @brief Request TDMA state from ISM module
 */
bool ism_request_state(void);

uint32_t ism_get_uart_rx_counter(void);

uint8_t ism_get_channels_size(uint8_t phy);

bool send_command(const uint8_t* data);
#endif
