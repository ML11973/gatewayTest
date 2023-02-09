#include "ism3_handlers.h"

// BEGIN USER INCLUDES (application-specific)
#include "main.h"
// END USER INCLUDES

// rx_unicast_function the function called when unicast are received
void rx_unicast_handler(const uint8_t* data, uint8_t size, uint8_t source, int8_t rssi, uint8_t lqi){
    #ifdef DEBUG_ISM3_HANDLERS
    printf("Handlers: RX unicast\n");
    #endif

    dispatchRxFrames(data, size, source);
	return;
}

// rx_multicast_function the function called when multicast are received
void rx_multicast_handler(const uint8_t* data, uint8_t size, uint8_t source, uint8_t countdown, int8_t rssi, uint8_t lqi){
    #ifdef DEBUG_ISM3_HANDLERS
    printf("Handlers: RX multicast\n");
    #endif

    dispatchRxFrames(data, size, source);
	return;
}

// beacon_data_function the function called when beacon data are received
void beacon_data_handler(const uint8_t* data, uint8_t size){
    #ifdef DEBUG_ISM3_HANDLERS
    printf("Handlers: beacon data change\n");
    #endif
	return;
}

// @param state_function the function called when the state change
void state_handler(ism_state_t state, const uint8_t* gateway_id){
    #ifdef DEBUG_ISM3_HANDLERS
    printf("Handlers: state change : ");
    #endif
	switch(state){
	case ISM_OFF:
        #ifdef DEBUG_ISM3_HANDLERS
		printf("ISM_OFF\n");
        #endif
        break;
	case ISM_NOT_SYNCHRONIZED:
		// Waiting for sync
        #ifdef DEBUG_ISM3_HANDLERS
		printf("ISM_NOT_SYNCHRONIZED\n");
        #endif
        break;
	case ISM_SYNCHRONIZED:
		// Comes here on wakeup from gateway
        #ifdef DEBUG_ISM3_HANDLERS
		printf("ISM_SYNCHRONIZED\n");
        #endif
        break;
	case ISM_LOW_POWER_SYNC:
		// Goes there when synced to gateway in SM_LOW_POWER and SM_LOW_POWER_GROUP, but not woken up
        #ifdef DEBUG_ISM3_HANDLERS
		printf("ISM_LOW_POWER_SYNC\n");
        #endif
        break;
	case ISM_TX_SYNC:
        // Goes there in gateway mode
        #ifdef DEBUG_ISM3_HANDLERS
		printf("ISM_TX_SYNC\n");
        #endif
        break;
	case ISM_VERSION_READY:
        // Goes there after first init
        #ifdef DEBUG_ISM3_HANDLERS
		printf("ISM_VERSION_READY\n");
        #endif
        break;
	default:
        break;
	}
	return;
}

// @param stat_function the function called when stat are read
void stat_handler(ism_stat_t stat){
    #ifdef DEBUG_ISM3_HANDLERS
    printf("Handlers: stats\n");
    #endif
	return;
}
