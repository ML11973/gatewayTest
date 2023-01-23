#include "ism3_server.h"
#include <stdio.h> // debug print


// Private config
const uint8_t servAddr=0x00;
const uint32_t group = 0xFFFFFFFF;
uint8_t power = 0x10; // lower power for USB usage
uint8_t power_dbm = 12;
const uint8_t phy = 0x00; // PHY = ETSI
uint8_t channels[256] = {0};
uint64_t gateway_beacon_id = 0;//0xD322FE7D02D3D117;



static uint32_t wokenGroups=0;

// PRIVATE FUNCTION PROTOTYPES
/**
 * @brief wake groups in wokenGroups
 * @return true if command was sent to ISM
 * Uses wokenGroups variable as group bitfield to wake
 * Set variable in public functions defined in ism_server.h
 */
bool ism_server_set_awake_groups();


// PUBLIC FUNCTION IMPLEMENTATIONS
void ism_server_init(uint8_t power_, uint8_t power_dbm_){
    power=power_;
    power_dbm = power_dbm_;

    // Handlers in ism3_handlers.h
    ism_init(   rx_unicast_handler,
                rx_multicast_handler,
                beacon_data_handler,
                state_handler,
                stat_handler);
    ism_set_phy(phy, channels);
    ism_config(servAddr, group, power, power_dbm, gateway_beacon_id);


    while(!ism_is_ready()){
        delayMs(10);
        ism_tick();
    }
    ism_set_sync_mode(SM_TX);
    for(uint8_t i=0;i<50;i++){
        delayMs(10);
        ism_tick();
    }
}

uint8_t ism_server_get_power(){
    return power;
}

uint8_t ism_server_get_power_dbm(){
    return power_dbm;
}

void ism_server_set_power(uint8_t newPower, uint8_t newPower_dbm){
    power=newPower;
    power_dbm=newPower_dbm;
    ism_config(servAddr, group, power, power_dbm, gateway_beacon_id);
    while(!ism_is_ready()){
        delayMs(10);
        ism_tick();
    }
    ism_set_sync_mode(SM_TX);
    for(uint8_t i=0;i<50;i++){
        delayMs(10);
        ism_tick();
    }
}

uint32_t ism_server_get_awake(){
    return wokenGroups;
}


bool ism_server_wakeup_group(uint32_t group){
    wokenGroups|=group;
    return ism_server_set_awake_groups();
}

bool ism_server_unwake_group(uint32_t group){
    wokenGroups&=(~group);
    return ism_server_set_awake_groups();
}

bool ism_server_unwake_groups(){
    wokenGroups=0;
    return ism_server_set_awake_groups();
}

bool ism_server_change_beacon_data(uint8_t * beaconData, uint8_t length){
    uint8_t * pFrame = NULL;
    bool result;

    pFrame = (uint8_t*)malloc(length+2);
    if(pFrame!=NULL){
        *pFrame = length+1; // size
        *(pFrame+1) = CMD_SET_SYNC_USER_DATA;
        memcpy(pFrame, beaconData, length);
        result=send_command(pFrame);
    }else{
        result=false;
    }
    free(pFrame);
    return result;
}

// PRIVATE FUNCTION IMPLEMENTATIONS
bool ism_server_set_awake_groups(){
    uint8_t frame[6] = {0};

    frame[0] = 5; // size
    frame[1] = CMD_SET_SYNC_TX_LOW_POWER;
    // Manual copy to ensure big endianness
    frame[5] = wokenGroups&0xFF;
    frame[4] = (wokenGroups>>8)&0xFF;
    frame[3] = (wokenGroups>>16)&0xFF;
    frame[2] = (wokenGroups>>24)&0xFF;

    return send_command(frame);
}
