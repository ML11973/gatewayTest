#include "menu.h"

const uint32_t timeoutMs = 5000;


int serverMenu();
int nodeMenu(Node & testNode_);
int powerNodeMenu(PowerNode & testNode_);
int wpanManagerMenu(wpanManager & gateway_);
int printMenu(string description, const vector<string> &items);

int mainMenu(PowerNode & testClient_, wpanManager & gateway_){

    vector<string> menuOptions {
        "Exit",
        "Server functions",
        "Node functions",
        "PowerNode functions",
        "WPAN manager functions"
    };
    int choice=printMenu("Main menu", menuOptions);
    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        while(serverMenu()){
            delayMs(20);
            ism_tick();
        };
        break;
    case 2:
        while(nodeMenu(testClient_)){
            delayMs(20);
            ism_tick();
        };
        break;
    case 3:
        while(powerNodeMenu(testClient_)){
            delayMs(20);
            ism_tick();
        };
        break;
    case 4:
        while(wpanManagerMenu(gateway_)){
            delayMs(20);
            ism_tick();
        }
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
        ism_server_change_beacon_data(&beaconData, beaconDataLength);
        beaconData++;
        break;
    case 4:
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

int nodeMenu(Node & testNode_){
    string input;
    vector<string> menuOptions {
        "Main menu",
        "Wake up node",
        "Sleep node",
        "Ping node",
        "TX 0x01 02 03 to node",
        "Get node UID",
        "Increment node address",
        "Print node info"
    };
    uint8_t txdata[]={1,2,3};
    uint8_t uid_buffer[NODE_UID8_WIDTH]={0};
    uint8_t newAddress=0;
    int choice=printMenu("Node menu", menuOptions);
    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        testNode_.wakeup();
        break;
    case 2:
        testNode_.sleep();
        break;
    case 3:
        testNode_.net_ping(timeoutMs);
        if(testNode_.pingStatus()==true)
            cout<<"Ping OK"<<endl;
        else
            cout<<"PING TIMEOUT"<<endl;
        break;
    case 4:
        testNode_.tx((uint8_t*)txdata, 3);
        break;
    case 5:
        testNode_.net_getUid(timeoutMs);
        if(testNode_.uidStatus()==true){
            //cout<<"UID OK"<<endl;
            testNode_.getUid(uid_buffer,NODE_UID8_WIDTH);
            cout << "Client UID: " << endl;
            for(uint8_t i=0;i<NODE_UID8_WIDTH;i++){
                cout<< to_string(uid_buffer[i]);
            }
            cout << endl;
        }else{
            cout<<"UID TIMEOUT"<<endl;
        }
        break;
    case 6:
        newAddress=testNode_.getAddr()+1;
        testNode_.net_setAddr(newAddress);
        // One tick is needed for the command to be sent
        ism_tick();
        testNode_.net_ping(timeoutMs);
        if(testNode_.pingStatus()==true){
            //cout<<"ADDR OK"<<endl;
            cout << "New address : " << to_string(testNode_.getAddr()) << endl;
        }else
            cout<<"ADDR TIMEOUT"<<endl;
        break;
    case 7:
        cout<<"client 1: "<<testNode_<<endl;
        return 0;
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}


int powerNodeMenu(PowerNode & testNode_){
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
        testNode_.wakeup();
        break;
    case 2:
        testNode_.net_ping(timeoutMs);
        if(testNode_.pingStatus()==true)
            cout<<"Ping OK"<<endl;
        else
            cout<<"PING TIMEOUT"<<endl;
        break;
    case 3:
        testNode_.app_getPower(timeoutMs);
        if(testNode_.getPowerStatus()==true){
            cout<< "Client power: " << to_string(testNode_.getPower()) << endl;
        }else{
            cout<<"GET POWER TIMEOUT"<<endl;
        }
        break;
    case 4:
        power=testNode_.getPower();
        testNode_.app_setPower(power+1, timeoutMs);
        if(testNode_.setPowerStatus()==true){
            cout<< "Client power: " << to_string(testNode_.getPower()) << endl;
        }else{
            cout<<"SET POWER TIMEOUT"<<endl;
        }
        break;
    case 5:
        testNode_.app_getPowerSetting(timeoutMs);
        if(testNode_.getPowerSettingStatus()==true){
            cout<< "Client power setting: " << to_string(testNode_.getPowerSetting()) << endl;
        }else{
            cout<<"GET POWER SETTING TIMEOUT"<<endl;
        }
        break;
    case 6:
        powerSetting=testNode_.getPowerSetting();
        testNode_.app_setPowerSetting(powerSetting+1,timeoutMs);
        if(testNode_.setPowerSettingStatus()==true){
            cout<< "Client power setting: " << to_string(testNode_.getPowerSetting()) << endl;
        }else{
            cout<<"SET POWER SETTING TIMEOUT"<<endl;
        }
        break;
    case 7:
        testNode_.app_getPowerSettings(timeoutMs);
        if(testNode_.getPowerSettingsStatus()==true){
            powerSettings=testNode_.getPowerSettingsList();
            cout<<"Client power settings size: ";
            cout<<to_string(testNode_.getPowerSettingsList().size())<<endl;
            cout<< "Client power settings list: "<<endl;
            for(uint8_t i=0;i<testNode_.getPowerSettingsList().size();i++){
                cout<<to_string(testNode_.getPowerSettingsList().at(i))<<" ";
            }
            cout << endl;
        }else{
            cout<<"GET POWER SETTINGS TIMEOUT"<<endl;
        }
        break;
    case 8:
        testNode_.app_getManifest(timeoutMs);
        if(testNode_.getManifestStatus()==true){
            manifest=testNode_.getManifest();
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

int wpanManagerMenu(wpanManager & gateway_){
    string input;
    vector<string> menuOptions {
        "Main menu",
        "Print static address table",
        "List reachable clients",
        "Dynamic node discovery",
    };
    int choice=printMenu("WPAN manager menu", menuOptions);
    switch(choice){
    case 0:
        return 0;
        break;
    case 1:
        gateway_.printNodes();
        break;
    case 2:
        //cout<<"client 1: "<<client<<endl;
        cout<<"Not implemented yet"<<endl;
        break;
    case 3:
        cout<<"Not implemented yet"<<endl;
        break;
    default:
        cout<<"Invalid choice"<<endl;
        break;
    }
    return 1;
}



int printMenu(string menuName, const vector<string> &items){
    string input;
    cout << "------------------------" << menuName << endl;
    for(int i=0;i<items.size();i++){
        cout<<to_string(i)<<"--"<<items[i]<<endl;
    }
    cout << "Choice : ";
    getline(cin,input);

    return stoi(input);
}
