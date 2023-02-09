/**
 * @file wpan.h
 * @brief Definitions for WPAN protocols
 */
#ifndef _WPAN_H
#define _WPAN_H

#include <stdbool.h>
//#define DEBUG_RXTX            ///< Print all RX/TX frames
//#define DEBUG_NET             ///< Print NET RX/TX frames
//#define DEBUG_APP             ///< Print APP_PROTOCOL RX/TX frames
#define DEBUG_DATA            ///< Print DATA_PROTOCOL RX/TX frames
//#define DEBUG_TICKS           ///< Print message for each WPAN manager tick
//#define DEBUG_WPANMANAGER     ///< Print debug messages for WPAN manager
#define DEBUG_DORA            ///< Print debug messages for DORA protocol
//#define DEBUG_DORA_FRAMES     ///< Print RX/TX DORA frames
//#define DEBUG_NODETYPES       ///< Print debug messages for node type management
//#define DEBUG_BORDERROUTER    ///< Print border router debug messages
#define DEBUG_CONNECTION      ///< Print connection debug messages
//#define DEBUG_SOCKETS         ///< Print internet socket debug messages
#define SHOW_TASKS ///< Print general console messages

#ifdef SHOW_TASKS
    #define DEBUG_BORDERROUTER
#endif

/**
 * @page protocols Node protocols
 * Protocols are consistently and simply framed like so:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Protocol ID
 * | 1      | 1      | Command
 * | 2      | X      | Command data
 *
 * This payload is inserted in the standard radio module frame, which contains
 * the message destination and length.
 *
 * Command data size is fixed in advance on a command-per-command basis. Nodes
 * are free to check for correct size or assume gateway and transmission are
 * perfect. Gateway checks for command data to be big enough, but typically does
 * not perform content checks.
 *
 * The following protocols have been implemented:
 * - @subpage net_protocol
 * - @subpage app_protocol
 * - @subpage app_err_protocol
 * - @subpage data_protocol
 */

/**
 * @page net_protocol Network protocol
 * This protocol contains all relevant commands to handle static and dynamic
 * addressing.
 *
 * Following commands exist:
 * - #NETWORK_PING
 * - #NETWORK_GETUID
 * - #NETWORK_SETADDR
 * - #NETWORK_SETGROUP
 * - #NETWORK_DISCONNECT
 * - #NETWORK_RENEW_LEASE
 * - #NETWORK_GET_PROTOCOLS
 *
 * @subpage dora_protocol are a little bit special and follow their
 * own format since they are broadcast to the network.
 */

/**
 * @brief Network protocol ID
 *
 * See @subpage net_protocol for details
 */
#define NETWORK_PROTOCOL_ID 0x01

/* NETWORK COMMANDS ***********************************************************/
/**
 * @brief Ping
 *
 * No command data. On reception, Node pings back with the same command.
 */
#define NETWORK_PING        0x01
/**
 * @brief Get UID
 *
 * Sent data: none.
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 12     | Node UID
 *
 * On reception, Node replies with its UID. UID byte order is little endian.
 */
#define NETWORK_GETUID      0x02
/**
 * @brief Set node address
 *
 * Sent data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Node new address
 *
 * Node reply: None, command is unconfirmed to account for address change time
 * on Node.
 */
#define NETWORK_SETADDR     0x04
/**
 * @brief Set node group
 *
 * Sent data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 4      | Node new group
 *
 * Node reply: None, command is unconfirmed to account for group change time
 * on Node.
 */
#define NETWORK_SETGROUP    0x05	// client changes group
/**
 * @brief Disconnect node
 *
 * No command data. On reception, Node confirms disconnect with the same
 * command, then resets its address and goes in dynamic addressing mode.
 * This default behavior allows easy reconnection, but can be excluded from
 * later Node remote implementations, so long as the disconnection is confirmed.
 */
#define NETWORK_DISCONNECT  0x08
/**
 * @brief Renew dynamic address lease
 *
 * Sent data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Address
 *
 * Gateway reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Lease duration
 *
 * Node asks gateway to renew its address lease for the address specified in
 * command data. Gateway confirms lease renewal with the new lease duration.
 * Lease is reset on reception of new lease duration.
 */
#define NETWORK_RENEW_LEASE 0x14
/**
 * @brief Get Node supported protocols
 *
 * Sent data: none
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Protocols
 *
 * Supported protocols are in a bit field.
 *
 * wpanManager asks node its supported protocols so it can pass this information
 * on to the user application.
 */
#define NETWORK_GET_PROTOCOLS 0x20

/**
 * @page dora_protocol Dynamic address management commands
 *
 * These commands are used as a sequence initiated by a Node configured in
 * dynamic addressing mode. This mode provides automatic configuration of Nodes
 * and does not require flashing different flavors of the same program on Nodes.
 *
 * The sequence follows the classic DHCP pattern of DORA, described in
 * <a href="https://en.wikipedia.org/wiki/Dynamic_Host_Configuration_Protocol#Operation"> this Wikipedia excerpt</a>.
 *
 * **Note:** Server and gateway are conceptually different but refer to the same
 * entity. Server refers to the DORA server, which attributes addresses, and
 * gateway is the broader WPAN manager and IP forwarder application.
 * Please do not be surprised to see these two terms get used interchangeably.
 *
 * Since Nodes start out technically address-less and default server address is
 * not specified, addressing conflicts could occur if a dynamically-addressed
 * Node's initial address were the same as a statically-addressed Node's.
 * Broadcasting DORA commands allows both server and client to pick their frames
 * up reliably. This however requires a second layer of addressing to
 * distinguish between concurrent DORA requests from different clients.
 *
 * As with MAC addressing in DHCP, this DORA implementation uses hardware
 * addresses to direct messages. The hardware address is the ISM3 unique ID.
 *
 * Consequently, DORA frames look a little bit different from other Network
 * protocol frames:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Protocol ID
 * | 1      | 1      | Command
 * | 2      | 12     | Source UID
 * | 14     | 12     | Destination UID
 * | 26     | X      | Command data
 *
 * See the relevant commands for command data information:
 * - #NETWORK_DISCOVER
 * - #NETWORK_OFFER
 * - #NETWORK_REQUEST
 * - #NETWORK_ACK
 * - #NETWORK_DORA_ERR
 *
 * To summarize:
 * 1. Node broadcasts a Discover command
 * 2. wpanManager responds with an Offer containing an address for the node
 * 3. Node Requests the address and the group it wants to be in.
 * 4. wpanManager Acknowledges the node's new address and group and gives it a
 * lease duration
 *
 * Lease duration is specified in minutes, depending on
 * #NETWORK_LEASE_UNIT_MINUTES. A lease of 0 minutes means the address was not
 * granted. Nodes are responsible for lease renewal, as WPAN manager will
 * consider them timed out once their lease expires.
 * A timed-out Node is not accessible anymore, it is considered disconnected.
 * A #NETWORK_DISCONNECT command is sent to timed-out nodes so that they may
 * know that they can no longer use their address.
 */
/**
 * @brief Discover command
 *
 * See @ref dora_protocol.
 *
 * Command data: none
 *
 * Node broadcasts this command to get an address offer from gateway.
 */
#define NETWORK_DISCOVER    0x10
/**
 * @brief Offer address
 *
 * See @ref dora_protocol.
 *
 * Command data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Address
 *
 * Gateway broadcasts this command to offer a node an address. Gateway should
 * choose a free address unless it wants a conflict.
 */
#define NETWORK_OFFER       0x11
/**
 * @brief Request address and group
 *
 * See @ref dora_protocol.
 *
 * Command data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Address
 * | 1      | 4      | Group
 *
 * Node broadcasts this command to request previously offered address from
 * gateway. Node also requests a group, typically to match its function. For
 * example, if the node is a power producer, it may want to be in the same
 * group as other power producers to facilitate communications to gateway.
 * Group is ultimately not a decisive factor and is up to the user.
 */
#define NETWORK_REQUEST     0x12
/**
 * @brief NETWORK_ACK
 *
 * See @ref dora_protocol.
 *
 * Command data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Address
 * | 1      | 4      | Group
 * | 5      | 1      | Lease duration
 *
 * Lease duration is 0 if NACK (gateway did not grant lease).
 * On reception, node is free to use the leased address and group for the
 * specified lease duration.
 * Node can optionnaly memorize the gateway's address from this frame.
 */
#define NETWORK_ACK         0x13    // server->client
/**
 * @brief Error command
 *
 * Command data: none
 *
 * DORA server can broadcast this frame to signal an error.
 */
#define NETWORK_DORA_ERR    0x18
/**
 * @brief Default group for DORA command transmission
 *
 * Gateway will wake up this group to allow nodes to get dynamic addresses.
 * In cases of channel overload by nodes looking to get an address, gateway
 * can put them to sleep with this group and prioritize traffic from already
 * configured nodes.
 */
#define NETWORK_DORA_GROUP      0x80000000
/**
 * @brief Default address for non-configured nodes
 *
 * Nodes can use this address as long as they did not get a lease from the
 * DORA server.
 */
#define NETWORK_NACK_BASE_ADDR  0x04
/**
 * @brief Lease to minute conversion
 *
 * Lease duration in minutes = #NETWORK_LEASE_UNIT_MINUTES * lease duration
 */
#define NETWORK_LEASE_UNIT_MINUTES 1

#define NETWORK_LEASE_WIDTH 1   ///< Width of network lease in DORA commands
#define NETWORK_UID8_WIDTH 12   ///< Width of ISM3 UID
#define NETWORK_ADDR_WIDTH 1    ///< Width of ISM3 addresses
#define NETWORK_GROUP_WIDTH 4   ///< Width of ISM3 group bitfield
#define NETWORK_DORA_MAX_FRAME_SIZE (2*NETWORK_UID8_WIDTH+NETWORK_ADDR_WIDTH+NETWORK_GROUP_WIDTH+NETWORK_LEASE_WIDTH)
///< Max DORA frame size excluding protocol ID and command. Based on ACK frame length.

#define NETWORK_MAX_FRAME_SIZE (sizeof(NETWORK_PROTOCOL_ID)+sizeof(NETWORK_PING)+NETWORK_DORA_MAX_FRAME_SIZE)
///< Max network frame size. Based on DORA ACK command length

/**
 * @page app_protocol Application protocol
 * This protocol contains relevant commands to handle a network of power
 * producers and consumers.
 *
 * Following commands exist:
 * - #APP_GETMANIFEST
 * - #APP_GETPOWER
 * - #APP_SETPOWER
 * - #APP_GETPOWERSETTING
 * - #APP_SETPOWERSETTING
 * - #APP_GETPOWERSETTINGS
 *
 * The idea behind this protocol is to provide simple means of getting and
 * setting power consumptions for different nodes. For easier identification,
 * a manifest can be obtained with the node's description.
 *
 * There are two ways to set a node's target power:
 * 1. Setting an absolute target
 * 2. Using a power setting
 *
 * Setting an absolute target is not always possible. Many devices control
 * power usage in steps. For example, an electric car charging station may have
 * several power modes corresponding to certain charge rates.
 * If the target use case is, for example, to regulate household total power
 * consumption, setting an unreachable target can cause some regulators to hang.
 *
 * To accomodate the impossibility to set arbitrary power targets, the present
 * protocol can transmit power settings. Power settings can represent anything
 * depending on target application, but they typically represent power levels
 * in kW. The application server first gets the power settings then can decide
 * which one to use depending on its needs.
 *
 * The following types can be changed to improve precision or reduce channel
 * usage:
 * - ::powerkW_t (default: float)
 * - ::powerTarget_t (default: float)
 * - ::powerSetting_t (default: uint8_t)
 *
 * General rules for node implementation:
 * - Nodes should allow a server to get the actual measured power consumption
 * with #APP_GETPOWER if it is available, even if target power is set with a
 * power setting.
 * - Nodes should not allow a server to set an arbitrary power if they cannot
 * set this power.
 * - Nodes can change the values behind their power settings during execution,
 * provided they communicate the change (for example with an unsollicited
 * #APP_GETPOWERSETTINGS).
 * - Node implementation can exclude some commands. If a node receives a command
 * it has no routine for, it should reply with #APP_ERR_NOCOMMAND.
 * This can be the case if a node cannot measure its power consumption.
 *
 */
/**
 * @brief Application protocol ID
 *
 * See @subpage app_protocol for details.
 */
#define APP_PROTOCOL_ID 0x10

/* APPLICATION COMMANDS *******************************************************/
#define APP_CMD_HEADER_SIZE 2   ///< (App protocol ID + App command) size


/**
 * @brief Get node manifest
 *
 * Sent data: none
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | priority
 * | 1      | 1      | n = description length
 * | 2      | n      | description
 *
 * Manifest contains a node's priority in power setting adjustments (see
 * sManifest) and its string description. Limited by #APP_MAX_DESC_LENGTH
 */
#define APP_GETMANIFEST         0x01
/**
 * @brief Get current node power
 *
 * Sent data: none
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | size of ::powerkW_t | measured power
 */
#define APP_GETPOWER            0x02
/**
 * @brief Set node target power
 *
 * Sent data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | size of ::powerkW_t | target power
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | size of ::powerkW_t | measured power
 */
#define APP_SETPOWER            0x03
/**
 * @brief Get node current power setting
 *
 * Sent data: none
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | size of ::powerSetting_t | current power setting
 */
#define APP_GETPOWERSETTING     0x04
/**
 * @brief Set node power setting
 *
 * Sent data:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | size of ::powerSetting_t | target power setting
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | size of ::powerSetting_t | current power setting
 */
#define APP_SETPOWERSETTING     0x05
/**
 * @brief Get node power settings list
 *
 * Sent data: none
 *
 * Node reply:
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | n = number of power settings
 * | 1      | 1      | w = length of one power setting (size of ::powerTarget_t )
 * | 2      | n*w    | n available power settings of w size
 *
 * Set n and w by changing ::powerTarget_t.
 * Use sPowerSettings for easy transmission.
 */
#define APP_GETPOWERSETTINGS    0x08

#define APP_MANIFEST_MINWIDTH   2 ///< Minimal size of a sManifest type
#define APP_MAX_POWER_SETTINGS 	((ISM_MAX_DATA_SIZE-APP_CMD_HEADER_SIZE)/sizeof(powerSetting_t))
///< Max number of power settings to fit in one frame
#define APP_MAX_DESC_LENGTH		((ISM_MAX_DATA_SIZE-APP_CMD_HEADER_SIZE-APP_MANIFEST_MINWIDTH)) ///< Max description length to fit in one frame

// TODO retest this part
typedef float powerkW_t;        ///< Type of raw power measurement
typedef float powerTarget_t;    ///< Type of target setting power values
typedef uint8_t powerSetting_t; ///< Type of power setting index

/**
 * @brief power settings
 *
 * Wrapper structure to conveniently send and receive power settings.
 */
typedef struct{
    uint8_t nPowerSettings;         ///< Number of power settings
    uint8_t powerSettingWidth;	    ///< Length of one power setting (float=4)
    powerTarget_t * powerSettingskW;///< Power setting array, could be switched to void * type
}sPowerSettings;

/**
 * @brief power node description
 *
 * Valuable information for a household power regulation application
 */
typedef struct{
    uint8_t priority;           ///< Lower is more priority, 0=do not adjust
	uint8_t descriptionLength;  ///< Node description length
    char * description;         ///< Node description
}sManifest;


/**
 * @page app_err_protocol Application error protocol
 *
 * This protocol allows exchange of error messages regarding application
 * commands. There is no command data.
 *
 * Following messages exist:
 * - #APP_ERR_NOCOMMAND
 * - #APP_ERR_INVALIDINDATA
 * - #APP_ERR_OUTDATATOOBIG
 * - #APP_ERR_NODEMEM
 *
 * Whether a node or a gateway actually implement these error messages is up
 * to the user.
 * Current PowerNode implementation only prints node error messages.
 */

/* APPLICATION ERROR COMMANDS *************************************************/
/**
 * @brief Application error protocol ID
 *
 * See @subpage app_err_protocol for details.
 */
#define APP_ERR_PROTOCOL_ID 0x18

#define APP_ERR_CMD_HEADER_SIZE APP_CMD_HEADER_SIZE ///< See #APP_CMD_HEADER_SIZE
#define APP_ERR_NOCOMMAND		0x01 ///< App command is not supported
#define APP_ERR_INVALIDINDATA	0x02 ///< Invalid received app command data
#define APP_ERR_OUTDATATOOBIG	0x03 ///< Invalid transmitted app command data
#define APP_ERR_NODEMEM			0x04 ///< Node could not access requested data

/**
 * @page data_protocol Data transfer protocol
 *
 * This protocol allows exchange of UDP datagrams between node and gateway.
 * It does not have commands.
 *
 * The communicators split the *datagram* in *packets* small enough for
 * transmission using ISM3 module.
 * This maximal size is defined by #DATA_MAX_PACKET_LENGTH.
 *
 * The packet header allows reconstruction of the original datagram if all
 * packets were received.
 * No error detection is performed.
 * Datagrams with missing packets are marked as valid unless the last packet is
 * missing. In the latter case, they are open to completion and should be
 * cleared periodically.
 *
 * Frame definition :
 *
 * | Offset | Length | Data
 * | :----: | :----: | ----
 * | 0      | 1      | Protocol ID: #DATA_PROTOCOL_ID
 * | 1      | m      | Datagram ID
 * | 1+m    | n      | Current packet index
 * | 1+m+n  | n      | Last packet index
 * | 1+m+2*n| X      | raw data
 *
 * n = size of ::packet_index_t
 *
 * m = size of ::datagram_id_t
 *
 * Max value of X is defined in #DATA_MAX_PACKET_LENGTH.
 * It depends on ::packet_index_t and ::datagram_id_t size.
 * Default implementation has m=n=1.
 * In practice, it is impossible at the moment to reliably transmit more than
 * 3 consecutive packets, limiting datagram size to 705.
 * This limit is probably sufficient for our low-power, low-data rate
 * application.
 * It could probably pointlessly be surpassed with a different PHY layer.
 */
typedef unsigned char packet_index_t; ///< Packet index type
typedef unsigned char datagram_id_t;  ///< Datagram ID type

/**
 * @brief local datagram
 *
 * This data type is ready to be filled with Data packets. Enough memory should
 * be allocated to contain all expected packets for the datagram.
 * Size should be updated on last packet reception to reflect actual data size.
 */
typedef struct{
    bool ready;         ///< Whether the datagram has received its last packet
    datagram_id_t id;   ///< Datagram ID
    uint16_t size;      ///< Data size
    uint8_t * data;     ///< Pointer to data
}sDatagram;

#define DATA_PROTOCOL_ID 0x20   ///< Data protocol ID

#define DATA_HEADER_SIZE (sizeof(datagram_id_t)+2*sizeof(packet_index_t))
///< Data frame header size
#define DATA_MAX_PACKET_LENGTH (ISM_MAX_DATA_SIZE-DATA_HEADER_SIZE-1)
///< Maximal number of bytes to fit in a data packet
#define DATA_PACKET_MAX_NUMBER (0x1 << (sizeof(packet_index_t)*8))
///< Maximal number of packets depending on packet_index_t max value
#define DATA_MAX_DATAGRAM_LENGTH (DATA_PACKET_MAX_NUMBER*DATA_MAX_PACKET_LENGTH-1)
///< Maximal datagram length depending on packet capacity and max number


#endif // _WPAN_H
