#include "powernode.h"

using namespace std;

PowerNode::PowerNode():Node(){
    protocols=NETWORK_PROTOCOL_ID | APP_PROTOCOL_ID | APP_ERR_PROTOCOL_ID/* | DATA_PROTOCOL_ID*/;
}

PowerNode::PowerNode(uint8_t _address, uint32_t _group) : Node(_address,_group){
    protocols=NETWORK_PROTOCOL_ID | APP_PROTOCOL_ID | APP_ERR_PROTOCOL_ID/* | DATA_PROTOCOL_ID*/;
}

PowerNode::PowerNode(uint8_t _address, uint32_t _group, uint8_t _leaseDuration)
: Node(_address,_group, _leaseDuration){
    protocols=NETWORK_PROTOCOL_ID | APP_PROTOCOL_ID | APP_ERR_PROTOCOL_ID/* | DATA_PROTOCOL_ID*/;
}

PowerNode::PowerNode(Node & base):PowerNode(base.getAddr(),base.getGroup(),base.getLeaseDuration()){
    leaseStartTime=base.getLeaseStartTime();
}

PowerNode::~PowerNode()
{
    free(manifest.description);
    free(powerSettings.powerSettingskW);
}


ostream& operator<<(ostream & out, const PowerNode & powerNode){
    const Node & node=powerNode;

    out<<node<<" Type : Power node Description: "<<powerNode.description;
    return out;
}

void PowerNode::show(){
    cout<<"Address: "<<to_string(getAddr());
    cout<<" Group: "<<to_string(getGroup());
    cout<<" Type : Power node Description: "<<description;
}

uint8_t PowerNode::getNodeTypeProtocols(){
    return NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID/*|UDP_PROTOCOL_ID*/;
}

vector<powerTarget_t> PowerNode::getPowerSettingsList(){
    return powerSettingsList;
}
powerkW_t PowerNode::getPower(){
    return powerkW;
}
powerSetting_t PowerNode::getPowerSetting(){
    return powerSetting;
}
sManifest PowerNode::getManifest(){
    return manifest;
}
string PowerNode::getDescription(){
    return description;
}


bool PowerNode::app_getPowerSettings(uint32_t timeoutMs){
    // Classic application frame
    uint8_t frame[2] = {APP_PROTOCOL_ID, APP_GETPOWERSETTINGS};
    printAppFrame(frame,2,true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &powerSettingsCallback);

    return powerSettingsCallback;
}

bool PowerNode::app_getPower(uint32_t timeoutMs){
    // Classic application frame
    uint8_t frame[2] = {APP_PROTOCOL_ID, APP_GETPOWER};
    printAppFrame(frame,2,true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &powerGetCallback);

    return powerGetCallback;
}

bool PowerNode::app_setPower(powerkW_t newPower, uint32_t timeoutMs){
    // Classic application frame
    uint8_t frame[2+sizeof(newPower)] = {0};
    frame[0]=APP_PROTOCOL_ID;
    frame[1]=APP_SETPOWER;
    if(memcpy(&frame[2],&newPower,sizeof(newPower))!=NULL){
        printAppFrame(frame,2+sizeof(newPower),true);
        // Send frame with timeout
        txTimeout(frame, 2+sizeof(newPower), timeoutMs, &powerSetCallback);
    }else{
        // Could not copy new power
    }
    return powerSetCallback;
}

bool PowerNode::app_getPowerSetting(uint32_t timeoutMs){
    // Classic application frame
    uint8_t frame[2] = {APP_PROTOCOL_ID, APP_GETPOWERSETTING};
    printAppFrame(frame,2,true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &powerSettingGetCallback);

    return powerSettingGetCallback;
}

bool PowerNode::app_setPowerSetting(powerSetting_t newPowerSetting, uint32_t timeoutMs){
    // Classic application frame
    uint8_t frame[2+sizeof(newPowerSetting)] = {0};
    frame[0]=APP_PROTOCOL_ID;
    frame[1]=APP_SETPOWERSETTING;
    frame[2]=newPowerSetting;

    printAppFrame(frame,2+sizeof(newPowerSetting),true);
    txTimeout(frame, 2+sizeof(newPowerSetting), timeoutMs, &powerSettingSetCallback);

    return powerSettingSetCallback;
}

bool PowerNode::app_getManifest(uint32_t timeoutMs){
    // Classic application frame
    uint8_t frame[2] = {APP_PROTOCOL_ID, APP_GETMANIFEST};
    printAppFrame(frame,2,true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &manifestCallback);

    return manifestCallback;
}

// CALLBACK FLAG GETTERS
bool PowerNode::getManifestStatus(){
    return manifestCallback;
}
bool PowerNode::setPowerStatus(){
    return powerSetCallback;
}
bool PowerNode::getPowerStatus(){
    return powerGetCallback;
}
bool PowerNode::setPowerSettingStatus(){
    return powerSettingSetCallback;
}
bool PowerNode::getPowerSettingStatus(){
    return powerSettingGetCallback;
}
bool PowerNode::getPowerSettingsStatus(){
    return powerSettingsCallback;
}



void PowerNode::appCmdCallback(const uint8_t* data, uint8_t size){
    uint8_t cmd = data[0];
    uint8_t dataSize = size-1;
    uint8_t dataIndex = 1;
    printAppFrame(data,size,false);
    if(dataSize>=1){
        switch(cmd){
            case APP_GETMANIFEST:
                if(copyManifest(&data[dataIndex],dataSize)>0)
                    manifestCallback=true;
                break;
            case APP_GETPOWER:
                powerGetCallback=getSetPowerHandler(&data[dataIndex],dataSize);
                break;
            case APP_SETPOWER:
                powerSetCallback=getSetPowerHandler(&data[dataIndex],dataSize);
                break;
            case APP_GETPOWERSETTING:
                powerSettingGetCallback=getSetPowerSettingHandler(&data[dataIndex],dataSize);
                break;
            case APP_SETPOWERSETTING:
                powerSettingSetCallback=getSetPowerSettingHandler(&data[dataIndex],dataSize);
                break;
            case APP_GETPOWERSETTINGS:
                if(copyPowerSettings(&data[dataIndex], dataSize)>0)
                    powerSettingsCallback=true;
                break;

            default:

                break;
        }
    }else{
        // Data too short
    }
}

void PowerNode::appErrCallback(const uint8_t * data, uint8_t size){
#ifdef DEBUG_APP
    cout<<"ERROR: Application protocol: ";
    switch(data[0]){
        case APP_ERR_NOCOMMAND:
            cout<<"No command";
            break;
        case APP_ERR_INVALIDINDATA:
            cout<<"Invalid input data";
            break;
        case APP_ERR_OUTDATATOOBIG:
            cout<<"Received data too big";
            break;
        case APP_ERR_NODEMEM:
            cout<<"Distant node memory error";
            break;
        case APP_ERR_UNDEFINEDCMD:
            cout<<"Undefined command";
            break;
        default:
            cout<<"Undefined error (no valid error code)";
            break;
    }
    cout<<endl;
#endif
}


bool PowerNode::getSetPowerHandler(const uint8_t * buffer, uint8_t size){
    if(size>=sizeof(powerkW)){
        if(memcpy(&powerkW,buffer,sizeof(powerkW))!=NULL)
            return true;
    }
    return false;
}

bool PowerNode::getSetPowerSettingHandler(const uint8_t * buffer, uint8_t size){
    memcpy(&powerSetting,buffer,sizeof(powerSetting_t));
    return true;
}

uint8_t PowerNode::copyManifest(const uint8_t * buffer, uint8_t size){
    const uint8_t manifestHeader = 2;
    uint8_t copiedBytes=0;

    if(size>manifestHeader){
        manifest.priority=buffer[0];
        manifest.descriptionLength=buffer[1];
    }

    if(size-manifestHeader==manifest.descriptionLength){
        // Free memory before new malloc
        free(manifest.description);
        manifest.description=(char*)malloc(manifest.descriptionLength);

        if(memcpy(manifest.description, &buffer[2], manifest.descriptionLength)!=NULL)
            copiedBytes=manifest.descriptionLength;
    }else{
        free(manifest.description);
        char errorDescription[]="No description found";
        manifest.description=(char*) malloc(sizeof(errorDescription));

        if(memcpy(manifest.description, errorDescription, sizeof(errorDescription))!=NULL)
            copiedBytes=sizeof(errorDescription);
    }
    if(copiedBytes>0)
        description=string(manifest.description);

    return copiedBytes;
}


uint8_t PowerNode::copyPowerSettings(const uint8_t * buffer, uint8_t size){
    uint8_t copiedBytes=0;

    if(size>2){
        powerSettings.nPowerSettings=buffer[0];
        powerSettings.powerSettingWidth=buffer[1];
    }

    uint8_t settingsWidth=powerSettings.nPowerSettings
                            *powerSettings.powerSettingWidth;

        // Free memory before new malloc
    free(powerSettings.powerSettingskW);
    if(size-2==settingsWidth){
        powerSettings.powerSettingskW=(powerTarget_t*)malloc(settingsWidth);

        if(memcpy(powerSettings.powerSettingskW, &buffer[2], settingsWidth)!=NULL){
            copiedBytes=settingsWidth;
            // raw data is copied correctly (tested)
            powerSettingsList=sPowerSettingsToFloatVector(powerSettings);
        }
    } // if no good data, powerSettings is reset
    return copiedBytes;
}


vector<powerTarget_t> PowerNode::sPowerSettingsToFloatVector(sPowerSettings powerSettings_){
    vector<powerTarget_t> vector;
    powerTarget_t * pCurrent = powerSettings_.powerSettingskW;
    for(uint8_t i=0; i<powerSettings_.nPowerSettings;i++){
        vector.push_back(*pCurrent);
        pCurrent++;
    }
    return vector;
}

// Redefinition
void PowerNode::printAppFrame(const uint8_t * buffer, uint8_t size, bool dir){
#ifdef DEBUG_APP
    if(dir)
        cout<<"PowerNode APP TX: ";
    else
        cout<<"PowerNode APP RX: ";

    printBufferHex(buffer, size);
#endif
}
