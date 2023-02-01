/**
 ******************************************************************************
 * @file    ism3.c
 * @author  nicolas.brunner@heig-vd.ch, marc.leemann@master.hes-so.ch
 * @date    06-August-2018
 * @brief   Driver for the RM1S3
 ******************************************************************************
 * @copyright HEIG-VD
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <time.h>   // nanosleep for delays

#include "commands_RM1S3.h"
#include "framed_uart.h"
#include "ism3.h"
#include "hardware.h"
#include "util.h"

/* Private define ------------------------------------------------------------*/
//#define NDEBUG
#define RESET_DURATION            10
#define START_DURATION            500
#define FRAME_TIMEOUT             150
#define BAUDRATE_CHANGE_DURATION  5
/**
 * @brief Hardware serial link to ISM3 baudrate
 *
 * Must be lower than 30000 for using stop2 mode of RM1
 */
#define BAUDRATE       19200

#define TX_HEADER_SIZE   4
#define BROADCAST_HEADER_SIZE TX_HEADER_SIZE+4
#define RX_HEADER_SIZE   6
#define SOURCE_INDEX     2
#define DATA_SLOT_INDEX  3
#define RSSI_INDEX       4
#define LQI_INDEX        5

#define RX_MULTICAST_HEADER_SIZE     11
#define RX_MULTICAST_SOURCE_INDEX     2
#define RX_MULTICAST_GROUP_INDEX      3
#define RX_MULTICAST_COUNTDOWN_INDEX  7
#define RX_MULTICAST_RSSI_INDEX       9
#define RX_MULTICAST_LQI_INDEX       10
#define GROUP_SIZE                    4

#define MAX_COMMAND_SIZE 18
#define NUMBER_OF_RECONFIGURATION_CMD 9
#define STATE_UNINITIALIZED 0xFF

#define TX_STATUS_NONE     0
#define TX_STATUS_WAIT_ACK 1
#define TX_STATUS_ACK      2
#define TX_STATUS_TIMEOUT  3

#define DEFAULT_PHY1 0 // default phy (ETSI)
#define DEFAULT_PHY2 3 // default phy when the first one failed (FCC)
#define UNALLOWED_PHY1 1 // Phy use for the test bench
#define UNALLOWED_PHY2 2 // Phy use for the test bench

#define DEFAULT_POWER     0x06 // ~+14dBm
#define DEFAULT_POWER_DBM 14

/* Private typedef -----------------------------------------------------------*/

typedef const uint8_t commands_t[][MAX_COMMAND_SIZE];

/* Private variables ---------------------------------------------------------*/

#ifdef ISM_GPIO3
#define NUMBER_OF_GPIO 11
#else
#define NUMBER_OF_GPIO 9
#endif


#ifndef CLIENT_CONFIG

static commands_t configuration_commands = {
    //{0x04, CMD_SET_HOST_BAUDRATE,     0x00, 0x4E, 0x20}, // 20000
    {0x04, CMD_SET_HOST_BAUDRATE,     0x00, 0x4B, 0x00}, // 19200
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

///< This array holds the sequence of commands sent to the ISM3 module.

///< **If baudrate is modified here, #BAUDRATE macro should also be modified**.
///< Changing ISM3 baudrate via this array does not change host platform serial
///< baudrate.
#endif


#ifdef CLIENT_CONFIG
static commands_t configuration_commands = {
    //{0x04, CMD_SET_HOST_BAUDRATE,     		0x00, 0x4E, 0x20}, // 20000

	{0x01, CMD_GET_FIRMWARE_VERSION},
	{0x01, CMD_GET_HARDWARE_VERSION},
	{0x01, CMD_GET_UNIQUE_DEVICE_ID},
    {0x05, CMD_SET_PATTERN, 0x19, 0x17, 0x10, 0x25}, // pattern
	{0x01, CMD_GET_PHYS_CHANNELS_PLAN_SIZE},
	{0x09, CMD_SET_SYNC_BEACON_ID, 	  		0xD3, 0x22, 0xFE, 0x7D, 0x02, 0xD3, 0xD1, 0x17}, // Gateway ID
	//{0x02, CMD_SET_SYNC_MODE, 		  		0x01}, // sync mode rx active
	{0x02, CMD_SET_RF_PHY, 			  		0x00}, // phy ETSI
    {0x02, CMD_SET_TX_RETRY_COUNT,    		0x02}, // Tx retry count = 2
	{0x02, CMD_SET_TX_RETRY_RESTRICTION, 	0x00}, // Tx retry restriction = next cycle
    {0x02, CMD_SET_GPIO0_SIGNAL,      		0x01}, // SIG_SYNC_OUT to GPIO0
    {0x02, CMD_SET_GPIO1_SIGNAL,      		0x02}, // SIG_TIMESLOT_OUT to GPIO1
	{0x0A, CMD_SET_SYNC_RX, 0x01, 0x64, 0x01, 0x92, 0x00, 0xCA, 0x00, 0x03, 0xE8},
	// interval = 1, missMax = 100, initialTrack = 402ms, trackOn = 202ms, trackOff = 1s
	{0x02, CMD_SET_EVENT_INDICATION,  		1}, // Enable IND_TDMA_STATE_CHANGED
	{0x02, CMD_SET_RADIO_MODE, 				0x00}, // radio mode inactive

    {0x02, CMD_SET_RF_PHY,            		0x00}, // use phy as parameter
    {0x02, CMD_SET_ACTIVE_CHANNELS,   		0x00}, // use channels as parameter

    // Following parameters can change during execution, the numbers of commands should be set in NUMBER_OF_RECONFIGURATION_CMD
    {0x02, CMD_SET_SYNC_MODE,         		0x00}, // use sync_mode as parameter
    {0x06, CMD_SET_SYNC_RX_LOW_POWER, 		0x00, 0x00, 0x00, 0x00, 1}, // use group as parameter, sync interval = 1 (all beacons)
    {0x02, CMD_SET_RF_POWER,          		0x00}, // use power as parameter
    {0x02, CMD_SET_ADDRESS,           		0x00}, // use address as parameter
    {0x05, CMD_SET_GROUP,             		0x00, 0x00, 0x00, 0x00}, // use group as parameter
    {0x09, CMD_SET_ASSOCIATED_BEACON_ID, 	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // use associated_beacon_id as parameter
    {0x04, CMD_SET_DATA_SLOT_RANGE_TYPE, 	0, 254, 0}, // deactivate all data slots
    {0x03, CMD_SET_DATA_SLOT_TYPE,    		0x00, 0x00}, // dataslot set automatically by software
    {0x02, CMD_SET_RADIO_MODE,        		0x01}, // Start TDMA
    {0x00}
};
#endif


static ism_unicast_function_t rx_unicast_function;
static ism_multicast_function_t rx_multicast_function;
static ism_beacon_data_function_t beacon_data_function;
static ism_state_function_t state_function;
static ism_stat_function_t stat_function;
static uint8_t state = STATE_UNINITIALIZED;
static uint8_t inner_state;
static ism_state_t ism_state;
static bool wait_response = false;
static uint8_t address;
static uint32_t group = 0xFFFFFFFF;
static uint8_t power = DEFAULT_POWER;
static uint8_t power_dbm = DEFAULT_POWER_DBM;
static uint64_t associated_beacon_id;
static uint8_t uid[UID_SIZE];
static ism_sync_mode_t sync_mode = SM_RX_LOW_POWER_GROUP;
static uint8_t tx_status = TX_STATUS_NONE;
static bool need_reconfiguration = false;

static char firmware_version[32];
static char hardware_version[32];
static uint8_t phys_channels_size[256];
static uint8_t phy_count;

static uint32_t frame_error_counter = 0;
static uint32_t frame_ok_counter = 0;
static uint32_t uart_rx_counter = 0;

static uint8_t phy;
static uint8_t channels[16];

/* Private functions declaration ---------------------------------------------*/

static bool is_initialized(void);
static void framed_rx(const uint8_t* data, uint16_t size);
static void send_init_command(void);
static void set_default_phy(void);

/* Public functions ----------------------------------------------------------*/

void ism_init(
        ism_unicast_function_t rx_unicast_function_,
        ism_multicast_function_t rx_multicast_function_,
        ism_beacon_data_function_t beacon_data_function_,
        ism_state_function_t state_function_,
        ism_stat_function_t stat_function_)
{
    rx_unicast_function = rx_unicast_function_;
    rx_multicast_function = rx_multicast_function_;
    beacon_data_function = beacon_data_function_;
    state_function = state_function_;
    stat_function = stat_function_;

    state = 0;
    inner_state = 0;
    ism_state = ISM_NOT_SYNCHRONIZED;

    framed_uart_init(framed_rx, FRAME_TIMEOUT);
    framed_uart_start();
}

void ism_config(uint8_t address_, uint32_t group_, uint8_t power_, uint8_t power_dbm_, uint64_t associated_beacon_id_)
{
    assert(address != 0xFF);
    address = address_;
    group = group_;
    power = power_;
    power_dbm = power_dbm_;
    associated_beacon_id = associated_beacon_id_;

    if (is_initialized()) {
        inner_state = 0;
        state -= NUMBER_OF_RECONFIGURATION_CMD;
    } else {
        need_reconfiguration = true;
    }
}

void ism_get_config(uint8_t* address_, uint32_t* group_, uint8_t* power_, uint8_t* power_dbm_, uint64_t* associated_beacon_id_)
{
    *address_ = address;
    *group_ = group;
    *power_ = power;
    *power_dbm_ = power_dbm;
    *associated_beacon_id_ = associated_beacon_id;
}

void ism_get_uid(uint8_t* uid_, uint8_t uid_size_){
    if(uid_size_<=UID_SIZE)
        memcpy(uid_, uid, uid_size_);
    else
        memcpy(uid_, uid, UID_SIZE);
}

bool ism_set_phy(uint8_t phy_, const uint8_t* channels_)
{
    uint8_t size = ism_get_channels_size(phy_);
    // phy_ is valid if it's channels size is more than zero
    if (size > 0) {
        phy = phy_;

        uint16_t sum = 0;
        for (int i = 0; i < size; i++) {
            sum += channels_[i];
        }

        if (sum == 0) {
            // No channel are set => use default value with all channels in use
            memset(channels, 0xFF, size);
        } else {
            memcpy(channels, channels_, size);
        }
    }
    return size > 0;
}

void ism_disconnect(void)
{
    if (is_initialized()) {
        inner_state = 0;
        state -= NUMBER_OF_RECONFIGURATION_CMD + 3; // Stop TDMA and restart it
    }
}

void ism_set_sync_mode(ism_sync_mode_t mode)
{
    uint8_t buffer[3];

    assert(mode <= SM_RX_LOW_POWER_GROUP);

    if (sync_mode != mode) {
        if (is_initialized()) {
            buffer[0] = 2;
            buffer[1] = CMD_SET_SYNC_MODE;
            buffer[2] = mode;

            if (send_command(buffer)) {
                sync_mode = mode;
            }
        } else {
            need_reconfiguration = true;
            sync_mode = mode;
        }
    }
}

void ism_power_down(void){
    // No power down pin commandable on Shield
    // May be done using a power down command
    return;
}

void ism_tick(void)
{

    assert(state != STATE_UNINITIALIZED);
    framed_uart_tick();

    if (!wait_response) {
        if (!is_initialized()) {
            send_init_command();
        }
    }
}

bool ism_is_tx_pending(void){
    // No implementation
    return false;
}

bool ism_is_ready(void)
{
    return is_initialized() && !framed_uart_is_busy();
}

bool ism_tx(uint8_t destination, const uint8_t* data, uint8_t size)
{
    uint8_t buffer[ISM_MAX_DATA_SIZE + TX_HEADER_SIZE];

    if (is_initialized() && (ism_state > ISM_NOT_SYNCHRONIZED) && !ism_is_tx_pending() && (size > 0) && (size <= ISM_MAX_DATA_SIZE)) {
        tx_status = TX_STATUS_WAIT_ACK;

        buffer[0] = size + TX_HEADER_SIZE - 1;
        buffer[1] = CMD_SEND_UNICAST;
        buffer[2] = destination;
        buffer[3] = 0xFF; // no dataslot restriction
        memcpy(&buffer[TX_HEADER_SIZE], data, size);
        return send_command(buffer);
    }
    return false;
}

bool ism_broadcast(uint32_t group, uint8_t number, const uint8_t* data, uint8_t size)
{
    uint8_t buffer[ISM_MAX_DATA_SIZE + TX_HEADER_SIZE];

    if (is_initialized() && (ism_state > ISM_NOT_SYNCHRONIZED) && !ism_is_tx_pending() && (size > 0) && (size <= ISM_MAX_DATA_SIZE)) {
        tx_status = TX_STATUS_WAIT_ACK;

        buffer[0] = size + BROADCAST_HEADER_SIZE - 1;
        buffer[1] = CMD_SEND_MULTICAST;
        util_uint32_to_byte_array(group, &buffer[2]);
        buffer[6] = number==0?0:number-1;
        buffer[7] = 0xFF; // no dataslot restriction
        memcpy(&buffer[BROADCAST_HEADER_SIZE], data, size);
        return send_command(buffer);
    }
    return false;
}

uint8_t ism_get_max_data_size(void)
{
    return ISM_MAX_DATA_SIZE;
}

char* ism_get_firmware_version(void)
{
    return firmware_version;
}

uint32_t ism_get_firmware_version_value(void)
{
    int result;
    unsigned int a, b, c;

    result = sscanf(firmware_version, "RM1S3:%u.%u.%u", &a, &b, &c);
    if (result == 3) {
        return (a << 16) + (b << 8) + c;
    } else {
        return 0;
    }
}

char* ism_get_hardware_version(void)
{
    return hardware_version;
}

bool ism_request_stat(void)
{
    uint8_t buffer[2];

    if (is_initialized()) {
        buffer[0] = 1;
        buffer[1] = CMD_GET_STAT;

        return send_command(buffer);
    }
    return false;
}

bool ism_request_state(void)
{
    uint8_t buffer[2];

    if (is_initialized()) {
        buffer[0] = 1;
        buffer[1] = CMD_GET_PROTOCOL_STATE;

        return send_command(buffer);
    }
    return false;
}

// No implementation
bool ism_update_firmware(const uint8_t* firmware, uint32_t size){
    return false;
}

uint32_t ism_get_uart_rx_counter(void)
{
    return uart_rx_counter;
}

uint8_t ism_get_channels_size(uint8_t phy)
{
    if (phy < phy_count) {
        return phys_channels_size[phy];
    } else {
        return 0;
    }
}


bool send_command(const uint8_t* data)
{
    bool result = framed_uart_tx(data, data[0] + 1);
    if (result) {
        wait_response = true;
    }
    return result;
}


void EXTI15_10_IRQHandler(void)
{
    //HAL_GPIO_EXTI_IRQHandler(((pin_t)ISM_GPIO1).GPIO_Pin);
}

/* Private functions implementation ------------------------------------------*/

static bool is_initialized(void)
{
    assert(state != STATE_UNINITIALIZED);
    return (configuration_commands[state][0] == 0);
}

static void send_init_command(void)
{
    static uint8_t command[MAX_COMMAND_SIZE];

    memcpy(command, configuration_commands[state], configuration_commands[state][0] + 1);

    if (command[1] == CMD_SET_RF_PHY) {
        command[2] = phy;
    } else if (command[1] == CMD_SET_ACTIVE_CHANNELS) {
        uint8_t size = ism_get_channels_size(phy);
        command[0] = size + 1;
        memcpy(&command[2], channels, size);
    } else if (command[1] == CMD_SET_SYNC_MODE) {
        command[2] = sync_mode;
    } else if (command[1] == CMD_SET_SYNC_RX_LOW_POWER) {
        util_uint32_to_byte_array(group, &command[2]);
    } else if (command[1] == CMD_SET_ADDRESS) {
        command[2] = address;
    } else if (command[1] == CMD_SET_GROUP) {
        util_uint32_to_byte_array(group, &command[2]);
    } else if (command[1] == CMD_SET_ASSOCIATED_BEACON_ID) {
        memcpy(&command[2], &associated_beacon_id, 8);
    } else if (command[1] == CMD_SET_RF_POWER) {
        if (power != ISM_INVALID_POWER) {
            command[2] = power;
        } else {
            command[1] = CMD_SET_RF_POWER_DBM;
            command[2] = power_dbm;
        }
    } else if (command[1] == CMD_SET_DATA_SLOT_TYPE) {
        if (inner_state == 0) {
            // server slot
            command[2] = 0;
            command[3] = 0x01; // rx
        } else if (inner_state == 1) {
            // own slot
            command[2] = address;
            command[3] = 0x02; // tx
        }
    }

    send_command(command);
}

static void framed_rx(const uint8_t* data, uint16_t size)
{

    // timeout
    if (data[0] != (size - 1)) {
        return;
    } else {
        uart_rx_counter++;
    }

    if (!is_initialized()) {
        if ((data[0] == 1) && (data[1] == CMD_SET_HOST_BAUDRATE)) {
            framed_uart_set_baudrate(BAUDRATE);
            state++;
            wait_response = false;
            struct timespec delay;
            delay.tv_sec = 1;
            nanosleep(&delay, NULL);
            framed_uart_flush();
        } else if ((data[0] == 1) && (data[1] == CMD_SET_DATA_SLOT_TYPE)) {
            inner_state++;
            if (inner_state == 2) {
                state++;
            }
            wait_response = false;
        } else if ((data[0] == 1) && (data[1] == configuration_commands[state][1])) {
            state++;
            wait_response = false;
        } else if ((data[0] == 1) && (data[1] == CMD_SET_RF_POWER_DBM) && (configuration_commands[state][1] == CMD_SET_RF_POWER)) {
            state++;
            wait_response = false;
        } else if (data[1] == CMD_GET_FIRMWARE_VERSION) {
            state++;
            wait_response = false;
            uint8_t length = data[0] - 1;
            if (length > sizeof(firmware_version) - 1) {
                // limit length to avoid buffer overflow
                length = sizeof(firmware_version) - 1;
            }
            memcpy(firmware_version, &data[2], length);
            firmware_version[length] = 0;
        } else if (data[1] == CMD_GET_HARDWARE_VERSION) {
            state++;
            wait_response = false;
            uint8_t length = data[0] - 1;
            if (length > sizeof(hardware_version) - 1) {
                // limit length to avoid buffer overflow
                length = sizeof(hardware_version) - 1;
            }
            memcpy(hardware_version, &data[2], length);
            hardware_version[length] = 0;
            if (state_function != NULL) state_function(ISM_VERSION_READY, NULL);
        }else if (data[1] == CMD_GET_UNIQUE_DEVICE_ID) {
            state++;
            wait_response = false;
            uint8_t length = data[0] - 1;
            if (length > sizeof(uid) - 1) {
                // limit length to avoid buffer overflow
                length = sizeof(uid) - 1;
            }
            memcpy(uid, &data[2], length);
        } else if (data[1] == CMD_GET_PHYS_CHANNELS_PLAN_SIZE) {
            state++;
            wait_response = false;
            phy_count = data[0] - 1;
            memcpy(phys_channels_size, &data[2], phy_count);
            phys_channels_size[UNALLOWED_PHY1] = 0;
            phys_channels_size[UNALLOWED_PHY2] = 0;
            set_default_phy();
        } else if (data[1] == IND_ERROR) {
            wait_response = false;
        }

        // When initialization is done, check if there is a need to reconfigure
        if (is_initialized() && need_reconfiguration) {
            need_reconfiguration = false;
            inner_state = 0;
            state -= NUMBER_OF_RECONFIGURATION_CMD;
        }
    }

    // Response from CMD_SEND_DATA
    if ((data[0] == 2) && (data[1] == IND_ERROR)) {
        frame_error_counter++;
        tx_status = TX_STATUS_TIMEOUT + data[2];
        wait_response = false;
    } else if ((data[0] == 1) && (data[1] == CMD_SEND_UNICAST)) {
        // Frame transmitted to modem correctly
        frame_ok_counter++;
        tx_status = TX_STATUS_ACK;
        wait_response = false;
    } else if ((data[0] == 1) && (data[1] == CMD_SEND_MULTICAST)){
        frame_ok_counter++;
        tx_status = TX_STATUS_ACK;
        wait_response = false;
    }

    // Response from CMD_SET_SYNC_MODE
    if ((data[0] == 1) && (data[1] == CMD_SET_SYNC_MODE)) {
        wait_response = false;
    }

    // Response from CMD_GET_STAT
    if ((data[0] == sizeof(ism_stat_t) + 1) && (data[1] == CMD_GET_STAT)) {
        if (stat_function != NULL) {
            ism_stat_t stat;
            memcpy(&stat, &data[2], sizeof(stat)); // or use util_byte_array_to_uint32
            stat_function(stat);
        }
        wait_response = false;
    }

    // Response from CMD_GET_PROTOCOL_STATE
    if ((data[0] == 2) && (data[1] == CMD_GET_PROTOCOL_STATE)) {
        wait_response = false;
    }

    // Asynchronous frame
    if ((data[1] == IND_RECEIVED_UNICAST) && (size > RX_HEADER_SIZE)) {
        if (rx_unicast_function != NULL) rx_unicast_function(
                &data[RX_HEADER_SIZE], // data
                size - RX_HEADER_SIZE, // size
                data[SOURCE_INDEX],
                data[RSSI_INDEX],
                data[LQI_INDEX]);
    } else if ((data[1] == IND_RECEIVED_MULTICAST) && (size > RX_MULTICAST_HEADER_SIZE)) {
        if (rx_multicast_function != NULL) rx_multicast_function(
                &data[RX_MULTICAST_HEADER_SIZE], // data
                size - RX_MULTICAST_HEADER_SIZE, // size
                data[RX_MULTICAST_SOURCE_INDEX],
                data[RX_MULTICAST_COUNTDOWN_INDEX],
                data[RX_MULTICAST_RSSI_INDEX],
                data[RX_MULTICAST_LQI_INDEX]);
    } else if ((data[1] == IND_UPDATED_SYNC_DATA) && (size >= 2)) {
        if (beacon_data_function != NULL) beacon_data_function(&data[2], size - 2);
    } else if (data[1] == IND_TDMA_STATE_CHANGED) {
        if (size == 3) {
            ism_state = (ism_state_t)data[2];
            if (state_function != NULL) state_function(ism_state, NULL);
        } else if (size == 11) {
            ism_state = (ism_state_t)data[2];
            if (state_function != NULL) state_function(ism_state, &data[3]);
        }
    }
}

static void set_default_phy(void)
{
    if (ism_get_channels_size(DEFAULT_PHY1) > 0) {
        phy = DEFAULT_PHY1;
        memset(channels, 0xFF, ism_get_channels_size(DEFAULT_PHY1));
    } else {
        phy = DEFAULT_PHY2;
        memset(channels, 0xFF, ism_get_channels_size(DEFAULT_PHY2));
    }
}
