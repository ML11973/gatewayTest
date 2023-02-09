#include "menu.h"

/**
 * @file menu.cpp
 * @brief Test menu functions
 */

const uint32_t timeoutMs = 5000; ///< default timeout for node commands

/**
 * @brief Server test menu
 * @return 0 if coming back to main menu
 *
 * Allows testing ISM3 server-level functions.
 * Allows waking up, sleeping, changing beacon data and reconfiguring
 * the gateway.
 */
int serverMenu();
/**
 * @brief Node test menu
 * @param gateway_ reference to the gateway instance
 * @return 0 if coming back to main menu
 *
 * Allows testing Node class implementation.
 * Allows waking up, putting to sleep, transmitting data, and network protocol
 * commands testing.
 */
int nodeMenu(wpanManager & gateway_);
/**
 * @brief PowerNode test menu
 * @param gateway_ reference to the gateway instance
 * @return 0 if coming back to main menu
 *
 * Allows testing PowerNode class implementation. Allows testing all application
 * protocol commands.
 */
int powerNodeMenu(wpanManager & gateway_);
/**
 * @brief DataNode test menu
 * @param gateway_ reference to the gateway instance
 * @return 0 if coming back to main menu
 *
 * Allows testing DataNode class implementation. TX small, variable-size or max
 * size datagrams. Print RX datagrams.
 */
int dataNodeMenu(wpanManager & gateway_);
/**
 * @brief wpanManager test menu
 * @param gateway_ reference to the gateway instance
 * @return 0 if coming back to main menu
 *
 * Allows testing wpanManager. Tick, print connected node lists, start/stop
 * dynamic address attribution, update node types, disconnect all clients.
 */
int wpanManagerMenu(wpanManager & gateway_);
/**
 * @brief wpanManager headless test routine
 * @param gateway_ reference to the gateway instance
 * @return 0 if coming back to main menu
 *
 * Starts wpanManager and ticks forever. Dynamic addressing is enabled.
 * Allows debugging depending on enabled debug messages in wpan.h
 */
int wpanManagerHeadless(wpanManager & gateway_);
/**
 * @brief borderRouter headless test routine
 * @param gateway_ reference to the gateway instance
 * @return 0 if coming back to main menu
 *
 * Starts a borderRouter instance with dynamic addressing.
 * Allows debugging depending on enabled debug messages in wpan.h
 */
int borderRouterHeadless(wpanManager & gateway_);
/**
 * @brief show a menu composed of argument items and title
 * @param description menu title
 * @param items vector of menu entries
 * @return user choice
 */
int printMenu(string description, const vector<string> &items);

int mainMenu(wpanManager & gateway_){
    vector<string> menuOptions {
        "Exit",
        "Server functions",
        "Node functions",
        "PowerNode functions",
        "DataNode functions",
        "WPAN manager functions",
        "Headless mode",
        "Border router mode"
    };
    int choice=printMenu("Main menu", menuOptions);
    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        while(serverMenu()){
            gateway_.tick(20);
        };
        break;
    case 2:
        while(nodeMenu(gateway_)){
            gateway_.tick(20);
        };
        break;
    case 3:
        while(powerNodeMenu(gateway_)){
            gateway_.tick(20);
        };
        break;
    case 4:
        while(dataNodeMenu(gateway_)){
            gateway_.tick(20);
        }
        break;
    case 5:
        while(wpanManagerMenu(gateway_)){
            gateway_.tick(20);
        }
        break;
    case 6:
        wpanManagerHeadless(gateway_);
        break;
    case 7:
        borderRouterHeadless(gateway_);
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}

int serverMenu(){
    vector<string> menuOptions {
        "Main menu",
        "Wake up group 1",
        "Sleep group 1",
        "Wake up all groups",
        "Sleep all groups",
        "Change beacon data",
        "Reconfigure gateway"
    };
    const uint8_t beaconDataLength=1;
    static uint8_t beaconData=1;
    uint8_t power=0;
    uint8_t power_dbm=0;
    int choice = printMenu("Server menu", menuOptions);

    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        ism_server_wakeup_group(1);
        break;
    case 2:
        ism_server_unwake_group(1);
        break;
    case 3:
        ism_server_wakeup_group(0xFFFFFFFF);
        break;
    case 4:
        ism_server_unwake_groups();
        break;
    case 5:
        ism_server_change_beacon_data(&beaconData, beaconDataLength);
        beaconData++;
        break;
    case 6:
        power=ism_server_get_power();
        power_dbm=ism_server_get_power_dbm();
        ism_server_init(power, power_dbm);
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}

int nodeMenu(wpanManager & gateway_){
    Node & testNode=*(gateway_.getNodeList().front());
    string input;
    vector<string> menuOptions {
        "Main menu",
        "Wake up node",
        "Sleep node",
        "Ping node",
        "TX 0x01 02 03 to node",
        "Get node UID",
        "Increment node address",
        "Increment node group",
        "Disconnect node",
        "Get node supported protocols",
        "Print node info"
    };
    uint8_t txdata[]={1,2,3};
    uint8_t uid_buffer[NETWORK_UID8_WIDTH]={0};
    uint8_t newAddress=0;
    uint32_t newGroup=0;
    uint8_t protocols;

    gateway_.tick(20);
    int choice=printMenu("Node menu", menuOptions);
    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        testNode.wakeup();
        break;
    case 2:
        testNode.sleep();
        break;
    case 3:
        testNode.net_ping(timeoutMs);
        if(testNode.pingStatus()==true)
            cout<<"Ping OK"<<endl;
        else
            cout<<"PING TIMEOUT"<<endl;
        break;
    case 4:
        testNode.tx((uint8_t*)txdata, 3);
        break;
    case 5:
        testNode.net_getUid(timeoutMs);
        if(testNode.uidStatus()==true){
            //cout<<"UID OK"<<endl;
            testNode.getUid(uid_buffer,NETWORK_UID8_WIDTH);
            cout << "Client UID: " << endl;
            for(uint8_t i=0;i<NETWORK_UID8_WIDTH;i++){
                cout<< to_string(uid_buffer[i]);
            }
            cout << endl;
        }else{
            cout<<"UID TIMEOUT"<<endl;
        }
        break;
    case 6:
        newAddress=testNode.getAddr()+1;
        testNode.net_setAddr(newAddress);
        // One tick is needed for the command to be sent
        ism_tick();
        testNode.net_ping(timeoutMs);
        if(testNode.pingStatus()==true){
            //cout<<"ADDR OK"<<endl;
            cout << "New address: " << to_string(testNode.getAddr()) << endl;
        }else
            cout<<"ADDR TIMEOUT"<<endl;
        break;
    case 7:
        newGroup=testNode.getGroup()<<1;
        testNode.net_setGroup(newGroup);
        cout<<"New group: "<<to_string(testNode.getGroup())<<endl;
        break;
    case 8:
        testNode.wakeup();
        testNode.net_disconnect(timeoutMs);
        gateway_.tick(20);
        testNode.net_ping(timeoutMs);
        gateway_.tick(20);
        if(testNode.pingStatus()==false){
            cout<<"Disconnect OK"<<endl;
        }else
            cout<<"Disconnect NOK"<<endl;
        testNode.sleep();
        break;
    case 9:
        testNode.net_getProtocols(timeoutMs);

        gateway_.tick(20);
        protocols=testNode.getProtocols();
        cout<<"Node protocols: "<<endl;
        if(protocols&NETWORK_PROTOCOL_ID){
            cout<<"Network"<<endl;
        }
        if(protocols&APP_PROTOCOL_ID){
            cout<<"Application"<<endl;
        }
        if(protocols&APP_ERR_PROTOCOL_ID){
            cout<<"Application error"<<endl;
        }
        if(protocols&DATA_PROTOCOL_ID){
            cout<<"UDP"<<endl;
        }
        break;
    case 10:
        cout<<"Client 1: "<<testNode<<endl;
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}

int powerNodeMenu(wpanManager & gateway_){
    PowerNode & testNode=*(gateway_.getPowerNodeList().front());
    string input;
    vector<string> menuOptions {
        "Main menu",
        "Wake up node",
        "Ping node",
        "Get power node measured power",
        "Increment power node target power",
        "Get power node power setting",
        "Increment power node power setting",
        "Get power node power settings list",
        "Get power node manifest"
    };
    vector<powerTarget_t> powerSettings;
    sManifest manifest;
    powerSetting_t powerSetting=0;
    powerkW_t power=0;
    int choice=printMenu("Power node menu", menuOptions);
    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        testNode.wakeup();
        break;
    case 2:
        testNode.net_ping(timeoutMs);
        if(testNode.pingStatus()==true)
            cout<<"Ping OK"<<endl;
        else
            cout<<"PING TIMEOUT"<<endl;
        break;
    case 3:
        testNode.app_getPower(timeoutMs);
        if(testNode.getPowerStatus()==true){
            cout<< "Client power: " << to_string(testNode.getPower()) << endl;
        }else{
            cout<<"GET POWER TIMEOUT"<<endl;
        }
        break;
    case 4:
        power=testNode.getPower();
        testNode.app_setPower(power+1, timeoutMs);
        if(testNode.setPowerStatus()==true){
            cout<< "Client power: " << to_string(testNode.getPower()) << endl;
        }else{
            cout<<"SET POWER TIMEOUT"<<endl;
        }
        break;
    case 5:
        testNode.app_getPowerSetting(timeoutMs);
        if(testNode.getPowerSettingStatus()==true){
            cout<< "Client power setting: " << to_string(testNode.getPowerSetting()) << endl;
        }else{
            cout<<"GET POWER SETTING TIMEOUT"<<endl;
        }
        break;
    case 6:
        powerSetting=testNode.getPowerSetting();
        testNode.app_setPowerSetting(powerSetting+1,timeoutMs);
        if(testNode.setPowerSettingStatus()==true){
            cout<< "Client power setting: " << to_string(testNode.getPowerSetting()) << endl;
        }else{
            cout<<"SET POWER SETTING TIMEOUT"<<endl;
        }
        break;
    case 7:
        testNode.app_getPowerSettings(timeoutMs);
        if(testNode.getPowerSettingsStatus()==true){
            powerSettings=testNode.getPowerSettingsList();
            cout<<"Client power settings size: ";
            cout<<to_string(testNode.getPowerSettingsList().size())<<endl;
            cout<< "Client power settings list: "<<endl;
            for(uint8_t i=0;i<testNode.getPowerSettingsList().size();i++){
                cout<<to_string(testNode.getPowerSettingsList().at(i))<<" ";
            }
            cout << endl;
        }else{
            cout<<"GET POWER SETTINGS TIMEOUT"<<endl;
        }
        break;
    case 8:
        testNode.app_getManifest(timeoutMs);
        if(testNode.getManifestStatus()==true){
            manifest=testNode.getManifest();
            cout<<"Client manifest: "<<endl;
            cout<<"Client description: "<<manifest.description<<endl;
            cout<<"Client priority: "<<to_string(manifest.priority)<<endl;
        }else{
            cout<<"GET MANIFEST TIMEOUT"<<endl;
        }
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}

int dataNodeMenu(wpanManager & gateway_){
    vector<DataNode *> pDataNodes=gateway_.getDataNodeList();
    if(pDataNodes.empty()){
        cout<<"No data node found"<<endl;
        return 0;
    }
    DataNode & testNode=*(pDataNodes.front());

    string input;
    vector<string> menuOptions {
        "Main menu",
        "Print node info",
        "Wake up node",
        "Ping node",
        "TX small datagram",
        "TX variable size datagram",
        "TX max size datagram",
        "Print RX datagram"
    };
    int size=0;
    uint8_t txDatagram[DATA_MAX_DATAGRAM_LENGTH]={0};
    uint8_t rxDatagram[DATA_MAX_DATAGRAM_LENGTH]={0};
    uint16_t rxLength=0;


    int choice=printMenu("Data node menu", menuOptions);


    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        cout << "Node info: ";
        testNode.show();
        cout<<endl;
        break;
    case 2:
        testNode.wakeup();
        break;
    case 3:
        testNode.net_ping(timeoutMs);
        if(testNode.pingStatus()==true)
            cout<<"Ping OK"<<endl;
        else
            cout<<"PING TIMEOUT"<<endl;
        break;
    case 4:
        // Fill datagram
        size=DATA_MAX_PACKET_LENGTH-50;
        for(uint8_t i=0;i<size;i++){
            txDatagram[i]=i;
        }
        testNode.datagram_tx(txDatagram,size);
        cout<<"Datagram (length "<<to_string(size)<<") sent : ";
        printBufferHex(txDatagram,size);
        break;
    case 5:
        cout << "Length : ";
        getline(cin,input);

        size=stoi(input);
        for(int i=0;i<size;i++){
            txDatagram[i]=(uint8_t) i;
        }
        testNode.datagram_tx(txDatagram,(uint16_t)size);
        cout<<"Datagram (length "<<to_string(size)<<") sent : ";
        printBufferHex(txDatagram,size);
        break;
    case 6:
        size=DATA_MAX_DATAGRAM_LENGTH;
        for(int i=0;i<size;i++){
            txDatagram[i]=(uint8_t) i;
        }
        testNode.datagram_tx(txDatagram,(uint16_t)size);
        cout<<"Datagram (length "<<to_string(size)<<") sent : ";
        printBufferHex(txDatagram,size);
        break;
    case 7:
        for(int i=0;i<10;i++){
            gateway_.tick(20);
        }
        rxLength=testNode.readDatagram(rxDatagram,DATA_MAX_DATAGRAM_LENGTH);

        cout<<"RX datagram (length "<<to_string(rxLength)<<") : ";
        printBufferHex(rxDatagram,rxLength);
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}

int wpanManagerMenu(wpanManager & gateway_){
    string input;
    vector<string> menuOptions {
        "Main menu",
        "WPAN manager tick",
        "Print static address table",
        "Print global address table",
        "Start dynamic discovery",
        "Stop dynamic discovery",
        "Update node types",
        "Clear address table"
    };
    int choice=printMenu("WPAN manager menu", menuOptions);
    ism_tick();
    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        for(uint8_t i=0;i<255;i++){
            gateway_.tick(20);
        }
        break;
    case 2:
        gateway_.printStaticNodes();
        break;
    case 3:
        gateway_.printNodes();
        break;
    case 4:
        gateway_.startDynamicDiscovery();
        for(uint16_t i=0;i<500;i++){
            gateway_.tick(10);
        }
        cout<<endl;
        break;
    case 5:
        gateway_.stopDynamicDiscovery();
        for(int i=0;i<5;i++){
            gateway_.tick(20);
        }
        cout<<endl;
        break;
    case 6:
        gateway_.updateNodeTypes();
        break;
    case 7:
        gateway_.clearNodeLists();
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}

int wpanManagerHeadless(wpanManager & gateway_){
    gateway_.startDynamicDiscovery();

    while(1){
        gateway_.tick(10);
        if(gateway_.nodeListUpdated()){
            cout<<"Connected nodes"<<endl;
            gateway_.printNodes();
        }
    }
    return 0;
}

int borderRouterHeadless(wpanManager & gateway_){
    gateway_.startDynamicDiscovery();
    BorderRouter::getInstance().init(&gateway_);

    while(1){
        BorderRouter::getInstance().tick(10);
    }
    return 0;
}

int printMenu(string menuName, const vector<string> &items){
    string input;
    int choice=0;
    cout << "------------------------" << menuName << endl;
    for(int i=0;i<items.size();i++){
        cout<<to_string(i)<<"--"<<items[i]<<endl;
    }
    cout << "Choice : ";
    cin.clear();
    getline(cin,input);
    try{
        choice=stoi(input);
    }
    catch(...){
        choice=255;
    }
    return choice;
}
