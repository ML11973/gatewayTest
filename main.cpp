
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
#include <iostream>
#include <string>
#include "powernode.h"        // Client power node
#include "menu.h"
/* Private includes ----------------------------------------------------------*/

using namespace std;
/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
//#define DEBUG_ADDRESSLIST

/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
wpanManager * instance;
/* Private function prototypes -----------------------------------------------*/
/**
 * @brief dispatch RX frames to clients based on source address
 * @param RX frame data and size
 * @param source address
 * Called from rx_unicast_handler in ism3_handlers
 */
void dispatchRxFrames(const uint8_t * data, uint8_t size, uint8_t source);
vector<PowerNode> getStaticAddressList(string file);
/* Private user code ---------------------------------------------------------*/

void dispatchRxFrames(const uint8_t * data, uint8_t size, uint8_t source){
  instance->rxHandler(data, size, source);
  /*
    if(source==client.getAddr()){
        client.rxCallback(data,size);
    }else{
        // RX source undefined
    }*/
}

/**
 * @brief
 *
 */
vector<PowerNode> getStaticAddressList(string file){
	// Fetch list of node addresses on drive
	ifstream readfile(file);
	string line = "254 254";
    stringstream ss(line);

    int address;
    int group;
    vector<PowerNode> nodes;
    //nodes.reserve(255);

    if(readfile.is_open()){
        while(getline(readfile, line)){
            if(line[0] != '#'){
                // clear stringstream
                ss.str(string());
                // Get new line in there
                ss << line;
                ss >> address;
                ss >> group;
#ifdef DEBUG_ADDRESSLIST
                cout<<"Read line: address="<<to_string(address);
                cout<<", group="<<to_string(group)<<endl;
#endif
                if( address<=254 && address>0 &&
                    group  <=255 && group  >0){
                    // Will not work if PowerNode is not constant
                    // Frankly it should work if it is not constant, but it
                    // segfaults when accessing, even with .at()
                    const PowerNode temp((uint8_t) address, (uint8_t) group);
                    nodes.push_back(temp);

#ifdef DEBUG_ADDRESSLIST
                    cout<<"Added node with address : "<<to_string(address)<<", group : "<<to_string(group)<<endl;
                    cout<<"In vector: address: "<<to_string(nodes.back().getAddr())<<", group: "<<to_string(nodes.back().getGroup())<<endl;
#endif
                }
            }
        }
        readfile.close();
    }else{
        cout<<"Could not open node address file"<<endl;
    }
    return nodes;
}


/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    // Server configuration
    uint8_t serverPower = 0x10; // lower power for USB usage
    uint8_t serverPower_dbm = 12;

    // NODE LIST INIT
    // Get node address list as power nodes
    const string addressListPath="./nodes.txt";
    vector<PowerNode> powerNodes= getStaticAddressList(addressListPath);

    // Print first node info
    //cout<<powerNodes.front()<<endl;

    //cout << "PowerNode list size="<<to_string(powerNodes.size()) << endl;

    // Fill a Node vector with powerNodes to initialize gateway
    // We must declare as PowerNode and point to as Node to profit from
    // virtual function dynamic redefinition
    vector<Node*> nodes;
    for(uint8_t i=0; i<powerNodes.size(); i++){
        //cout<<powerNodes[i]<<endl;
      nodes.push_back(&powerNodes.at(i));
    }

    //cout<<*(nodes.front())<<endl;

    wpanManager gateway{nodes,serverPower,serverPower_dbm};
    cout << "ISM Init OK" << endl;
    instance=&gateway;
    // IMPORTANT: Declare a reference, not an object
    /* Object declaration will copy the PowerNode, so RX handler will
     * update flags in wpanManager's list and not touch the copy.
     * Menu functions operate by checking their argument client's
     * callback flags, so they will fail even though everything has gone
     * correctly
     */
    PowerNode & client=powerNodes.front();
    PowerNode & client2=powerNodes.at(1);
    //ism_server_init(serverPower, serverPower_dbm);


    cout << "ISM3 gateway test" << endl;
    cout << "Application layer test" << endl;
    cout << "Test client address: " << to_string(client.getAddr()) << endl;
    /* Menu loop */
    while(mainMenu(client, gateway)!=0)
    {

    }
    return 0;
}


