
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Port of test_choice.py
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/*
 * TODO:
 * Update config for server use as cleanly as possible
 * test ism3_server
 * test main from test_choice.py
 * investigate usleep in ism3.c
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
//#include <stdint.h>
//#include <stdio.h>
//#include <unistd.h>
//#include <time.h>

/* Private includes ----------------------------------------------------------*/

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define DATA_WIDTH 239

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

volatile uint8_t nFrames = 0;
uint8_t cliAddr = 0x08;
const uint8_t servAddr = 0x00;
uint32_t group = 0x1;// 0xFFFFFFFF;
uint8_t power = 0x10; // lower power for USB usage
uint8_t power_dbm = 12;
uint64_t associated_beacon_id = 0;//0xD322FE7D02D3D117;
uint64_t * pBeacon = &associated_beacon_id;
uint8_t phy = 0x00; // PHY = ETSI
uint8_t channels[256] = {0};

uint8_t txdata [DATA_WIDTH] = {0};
uint8_t rxdata [DATA_WIDTH] = {0};

/* Private function prototypes -----------------------------------------------*/

/* Private user code ---------------------------------------------------------*/

void delayMs(uint32_t milliseconds){
	struct timespec ms;
	ms.tv_sec = 0;
	ms.tv_nsec = milliseconds*1000*1000;
	nanosleep(&ms, NULL);
}

/* POWER MODES:
 *
 * SM_TX -> for gateway
 * SM_RX_ACTIVE -> reacts to beacon data changes, receives async rx unicast frames, node is in SYNCHRONIZED state
 * SM_RX_LOW_POWER -> no group wakeup, node receives beacon data changes, no async rx
 * SM_RX_LOW_POWER_GROUP -> allows wakeup from gateway, node wakes up in SYNCHRONIZED state, async RX when woken up
 *
 * approx. 15s for node to notify lost sync
 * STM32 sometimes doesn't call handler on ISM state change
 *
 * Working scenario:
 * - boot two modules
 * - wait for sync
 * - wake up group
 * - TX from client to gateway, gateway RX ok
 * - unwake group
 *
 * RX in SYNCHRONIZED works, but with a wait between frames
 * CHANGING beacon data from gateway will be acknowledged in SYNCHRONIZED and LOW_POWER_SYNC states at least
 */

/* GATEWAY COMMANDS:
 *
 */

// rx_unicast_function the function called when unicast are received
void rx_unicast(const uint8_t* data, uint8_t size, uint8_t source, int8_t rssi, uint8_t lqi){
	memcpy(rxdata, data, size);
	//ism_set_sync_mode(SM_RX_ACTIVE);
	//ism_tx(source, data, size);
	//ism_set_sync_mode(SM_RX_LOW_POWER_GROUP);
	nFrames++;
		printf("RX_UNICAST\n");
	return;
}

// rx_multicast_function the function called when multicast are received
void rx_multicast(const uint8_t* data, uint8_t size, uint8_t source, uint8_t countdown, int8_t rssi, uint8_t lqi){
	memcpy(rxdata, data, size);
	//ism_set_sync_mode(SM_RX_ACTIVE);
	//ism_tx(source, data, size);
	//ism_set_sync_mode(SM_RX_LOW_POWER);
		printf("RX_MULTICAST\n");
	return;
}

// beacon_data_function the function called when beacon data are received
void beacon_data(const uint8_t* data, uint8_t size){
	memcpy(rxdata, data, size);

		printf("BEACON DATA CHANGE\n");
	nFrames++;
	return;
}

// @param state_function the function called when the state change
void state(ism_state_t state, const uint8_t* gateway_id){
		printf("STATE CHANGE: ");
	switch(state){
	case ISM_OFF:
		  //ism_config(cliAddr, group, power, power_dbm, associated_beacon_id);
		printf("ISM_OFF\n");

	break;
	case ISM_NOT_SYNCHRONIZED:
		// Waiting for sync
		//memcpy(&associated_beacon_id, gateway_id, sizeof(associated_beacon_id));
		//ism_config(cliAddr, group, power, power_dbm, associated_beacon_id);
		//ism_set_sync_mode(SM_RX_LOW_POWER);
		printf("ISM_NOT_SYNCHRONIZED\n");
	break;
	case ISM_SYNCHRONIZED:
		// Comes here on wakeup from gateway
		memcpy(pBeacon, gateway_id, sizeof(associated_beacon_id));
		//ism_config(cliAddr, group, power, power_dbm, associated_beacon_id);
		//ism_set_sync_mode(SM_RX_ACTIVE);
		//ism_tx(servAddr, txdata, DATA_WIDTH);
		printf("ISM_SYNCHRONIZED\n");
		//HAL_Delay(50);
		//ism_set_sync_mode(SM_RX_LOW_POWER_GROUP);
	break;
	case ISM_LOW_POWER_SYNC:
		// Goes there when synced to gateway in SM_LOW_POWER and SM_LOW_POWER_GROUP, but not woken up
		//ism_set_sync_mode(SM_RX_ACTIVE);
		printf("ISM_LOW_POWER_SYNC\n");
	break;
	case ISM_TX_SYNC:
		//ism_set_sync_mode(SM_RX_ACTIVE);
		//ism_tx(servAddr, txdata, DATA_WIDTH);
		//ism_set_sync_mode(SM_RX_LOW_POWER);
		printf("ISM_TX_SYNC\n");

	break;
	case ISM_VERSION_READY:

		printf("ISM_VERSION_READY\n");
	break;
	default:
	break;
	}
	return;
}

// @param stat_function the function called when stat are read
void stat(ism_stat_t stat){
	nFrames++;
		printf("STAT\n");
	return;
}

int menu(){
    std::string input;
    std::string menuOptions[7] = {
        "Exit",
        "Wake up group",
        "Unwake group",
        "TX to client",
        "RX dump",
        "Change beacon data",
        "Reconfigure gateway"
    };
    char option='0';
    std::cout << "MENU-----------" << std::endl;
    for (std::string desc : menuOptions){
        std::cout << option << "--" << desc << std::endl;
        option++;
    }
    std::cout << "Choice : ";
    std::getline(std::cin,input);

    return input[0]-'0';
}

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    int menuChoice = 0;
    char beaconData = 1;
    size_t beaconDataLength = 1;

    std::cout << "ISM3 gateway test" << std::endl;

	for (uint8_t i=0; i<DATA_WIDTH; i++)
		txdata[i] = i;


  ism_init(rx_unicast, rx_multicast, beacon_data, state, stat);
  ism_config(servAddr, 0/*group*/, power, power_dbm, associated_beacon_id);


  for (uint16_t i=0; i<20; i++){
  	delayMs(10);
  	ism_tick();
  }
  ism_set_phy(phy, channels); // phy = ETSI
  for (uint16_t i=0; i<20; i++){
	delayMs(10);
	ism_tick();
  }

  ism_set_sync_mode(SM_TX);
  std::cout << "Init OK" << std::endl;

  /* Infinite loop */
  while (1)
  {
	  for (uint16_t i=0; i<100; i++){
	  	delayMs(10);
	  	ism_tick();
	  }

    menuChoice = menu();

    switch(menuChoice){
        case 0:
            exit(0);
            break;
        case 1:
            ism_server_wakeup_group(group);
            break;
        case 2:
            ism_server_unwake_groups();
            break;
        case 3:
            txdata[0] = 'T';
            txdata[1] = 'X';
            txdata[2] = '\0';
            ism_tx(cliAddr, (uint8_t*)txdata, 3);
            break;
        case 4:
            std::cout << "Do nothing" << std::endl;
            break;
        case 5:
            ism_server_change_beacon_data(&beaconData, beaconDataLength);
            beaconData++;
            break;
        case 6:
            ism_config(servAddr, 0/*group*/, power, power_dbm, associated_beacon_id);
            break;
        default:
            std::cout << "No selection" << std::endl;
            break;
    }
  }
}
