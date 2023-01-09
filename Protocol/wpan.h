#ifndef _NETWORK_H
#define _NETWORK_H

//#define DEBUG_RXTX  // Print all RX/TX frames
//#define DEBUG_NET   // Print NET RX/TX frames
//#define DEBUG_APP   // Print APP_PROTOCOL RX/TX frames

#define NETWORK_PROTOCOL_ID 0x01

// Network commands
#define NETWORK_PING    0x01
#define NETWORK_GETUID  0x02
#define NETWORK_SETADDR 0x04

// Application commands
#define APP_PROTOCOL_ID 0x10
#define APP_ERR_PROTOCOL_ID 0x11

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
#define APP_MAX_POWER_SETTINGS 	(ISM_MAX_DATA_SIZE-APP_CMD_HEADER_SIZE)/sizeof(powerSetting_t)
#define APP_MAX_DESC_LENGTH		(ISM_MAX_DATA_SIZE-APP_CMD_HEADER_SIZE-APP_MANIFEST_MINWIDTH)

typedef float powerkW_t;        // type of raw power measurement
typedef float powerTarget_t;    // type of target setting power values
typedef uint8_t powerSetting_t; // type of power setting index

typedef struct{
    uint8_t nPowerSettings;		// number of power settings
    uint8_t powerSettingWidth;	// length of one power setting (float=4)
    powerTarget_t * powerSettingskW;    // could be switched to void
}sPowerSettings;

typedef struct{
    uint8_t priority;           // lower is more priority, 0=do not adjust
    powerkW_t powerkW;              // negative means power is being produced
    sPowerSettings powerSettings;
	uint8_t descriptionLength;  // maybe useless TODO check and see
    char description[];
}sNodeReport;

typedef struct{
    uint8_t priority;           // lower is more priority, 0=do not adjust
	uint8_t descriptionLength;  // maybe useless TODO check and see
    char * description;
}sManifest;


#endif // _NETWORK_H
