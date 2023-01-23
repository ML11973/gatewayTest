#ifndef _WPAN_H
#define _WPAN_H

#include <stdbool.h>
//#define DEBUG_RXTX            // Print all RX/TX frames
//#define DEBUG_NET             // Print NET RX/TX frames
//#define DEBUG_APP             // Print APP_PROTOCOL RX/TX frames
#define DEBUG_DATA            // Print DATA_PROTOCOL RX/TX frames
//#define DEBUG_TICKS           // Print message for each WPAN manager tick
//#define DEBUG_WPANMANAGER     // Print debug messages for WPAN manager
#define DEBUG_DORA            // Print debug messages for DORA protocol
//#define DEBUG_DORA_FRAMES     // Print RX/TX DORA frames
//#define DEBUG_NODETYPES       // Print debug messages for node type management
//#define DEBUG_BORDERROUTER    // Print border router debug messages
#define DEBUG_CONNECTION      // Print connection debug messages
//#define DEBUG_SOCKETS         // Print internet socket debug messages
#define SHOW_TASKS

#ifdef SHOW_TASKS
    #define DEBUG_BORDERROUTER
#endif

#define NETWORK_PROTOCOL_ID 0x01

// Network commands
#define NETWORK_PING        0x01
#define NETWORK_GETUID      0x02
#define NETWORK_SETADDR     0x04
#define NETWORK_SETGROUP    0x05	// client changes group
#define NETWORK_DISCONNECT  0x08	// client confirms disconnect then resets address to dyn no lease
#define NETWORK_RENEW_LEASE 0x14	// client asks for lease renewal and gets confirmation

// DHCP-like address attribution
/* Frame data:
 * NETWORK_PROTOCOL_ID
 * NETWORK_CMD
 * source UID
 * destination UID
 * NETWORK_CMD_DATA
 *
 * NETWORK_CMD_DATA per command
 * DISCOVER:    none
 * OFFER:       address(1)
 * REQUEST:     address(1)  group(4)
 * ACK:         address(1)  group(4)    lease duration(1) (0 if NACK)
 *
 * Nodes are responsible for lease renewal.
 * If not renewed, node sends a NETWORK_DISCONNECT command
 */
#define NETWORK_DISCOVER    0x10    // client->server
#define NETWORK_OFFER       0x11    // server->client
#define NETWORK_REQUEST     0x12    // client->server
#define NETWORK_ACK         0x13    // server->client

#define NETWORK_NACK_GROUP      0x80000000
#define NETWORK_DORA_GROUP      0x80000000
#define NETWORK_NACK_BASE_ADDR  0x04

#define NETWORK_DORA_ERR    0x18    // server error notification
#define NETWORK_GET_PROTOCOLS 0x20 // Node should announce its supported protocols

#define NETWORK_LEASE_UNIT_MINUTES 1    // Lease=10->100 mn lease

#define NETWORK_LEASE_WIDTH 1 // 1 byte
#define NETWORK_UID8_WIDTH 12
#define NETWORK_ADDR_WIDTH 1
#define NETWORK_GROUP_WIDTH 4
#define NETWORK_DORA_MAX_FRAME_SIZE (2*NETWORK_UID8_WIDTH+NETWORK_ADDR_WIDTH+NETWORK_GROUP_WIDTH+NETWORK_LEASE_WIDTH)

#define NETWORK_MAX_FRAME_SIZE (sizeof(NETWORK_PROTOCOL_ID)+sizeof(NETWORK_PING)+NETWORK_DORA_MAX_FRAME_SIZE)

// Application commands
#define APP_PROTOCOL_ID 0x10
#define APP_ERR_PROTOCOL_ID 0x18

#define APP_CMD_HEADER_SIZE 2
#define APP_ERR_CMD_HEADER_SIZE APP_CMD_HEADER_SIZE

#define APP_GETMANIFEST         0x01
#define APP_GETPOWER            0x02
#define APP_SETPOWER            0x03
#define APP_GETPOWERSETTING     0x04
#define APP_SETPOWERSETTING     0x05
#define APP_GETPOWERSETTINGS    0x08

#define APP_ERR_NOCOMMAND		0x01
#define APP_ERR_INVALIDINDATA	0x02
#define APP_ERR_OUTDATATOOBIG	0x03
#define APP_ERR_NODEMEM			0x04
#define APP_ERR_UNDEFINEDCMD    0x05

#define APP_MANIFEST_MINWIDTH   2
#define APP_MAX_POWER_SETTINGS 	((ISM_MAX_DATA_SIZE-APP_CMD_HEADER_SIZE)/sizeof(powerSetting_t))
#define APP_MAX_DESC_LENGTH		((ISM_MAX_DATA_SIZE-APP_CMD_HEADER_SIZE-APP_MANIFEST_MINWIDTH))

typedef float powerkW_t;        // type of raw power measurement
typedef float powerTarget_t;    // type of target setting power values
typedef uint8_t powerSetting_t; // type of power setting index

typedef struct{
    uint8_t nPowerSettings;		// number of power settings
    uint8_t powerSettingWidth;	// length of one power setting (float=4)
    powerTarget_t * powerSettingskW;    // could be switched to void
}sPowerSettings;

// TODO move priority in sPowerSettings
typedef struct{
    uint8_t priority;           // lower is more priority, 0=do not adjust
	uint8_t descriptionLength;
    char * description;
}sManifest;


/*
 * DATA protocol:
 * Segments UDP datagrams into packets to be sent in radio frames
 *
 * Frame definition:
 * Protocol ID      Datagram ID   Packet index    Last packet index
 * Packet
 * TODO add source address and port in header for replies
 */
typedef unsigned char packet_index_t; // 8 bit subpacket indices
typedef unsigned char datagram_id_t;

typedef struct{
    bool ready;
    datagram_id_t id;
    uint16_t size;
    uint8_t * data;
}sDatagram;

#define DATA_PROTOCOL_ID 0x20

#define DATA_HEADER_SIZE (sizeof(datagram_id_t)+2*sizeof(packet_index_t))
#define DATA_MAX_PACKET_LENGTH (ISM_MAX_DATA_SIZE-DATA_HEADER_SIZE-1)
#define DATA_PACKET_MAX_NUMBER (0x1 << (sizeof(packet_index_t)*8))
#define DATA_MAX_DATAGRAM_LENGTH (DATA_PACKET_MAX_NUMBER*DATA_MAX_PACKET_LENGTH-1)


#endif // _WPAN_H
