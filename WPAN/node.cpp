#include "node.h"

Node::Node()
{
}


Node::Node(uint8_t _address, uint32_t _group)
{
    // Initialize node
    address=_address;
    oldAddress=_address;
    group=_group;
    leaseStartTime=(uint32_t) time(NULL);
}


Node::Node(uint8_t _address, uint32_t _group, uint8_t _leaseDuration)
:Node{_address,_group}{
    leaseDuration=_leaseDuration;
}


Node::~Node(){}


ostream& operator<<(ostream & out, const Node & node){
    out<<"Address: "<<to_string(node.address);
    out<<" Group: "<<to_string(node.group);
    return out;
}


void Node::show(){
    cout<<"Address: "<<to_string(address);
    cout<<" Group: "<<to_string(group);
}

// GETTERS

uint8_t Node::getUid(uint8_t * buffer, uint8_t size){
    if(size>=NETWORK_UID8_WIDTH){
        memcpy(buffer, uid, NETWORK_UID8_WIDTH);
        return NETWORK_UID8_WIDTH;
    }
    return 0;
}
uint8_t Node::getAddr(){
    return address;
}
uint8_t Node::getOldAddr(){
    return oldAddress;
}
uint32_t Node::getGroup(){
    return group;
}
uint8_t Node::getLeaseDuration(){
    return leaseDuration;
}
uint32_t Node::getLeaseStartTime(){
    return leaseStartTime;
}
uint8_t Node::getProtocols(){
    return protocols;
}

uint8_t Node::getNodeTypeProtocols(){
    return NETWORK_PROTOCOL_ID;
}

bool Node::wakeup(){
    if(group!=0)
        return ism_server_wakeup_group(group);
    else return false;
}

bool Node::sleep(){
    if(group!=0)
        return ism_server_unwake_group(group);
    else return false;
}

uint8_t Node::tx(uint8_t * buffer, uint8_t length){
    printFrame(buffer, length, true);
    return ism_tx(address, buffer, length);
}


bool Node::txTimeout(uint8_t * frame, uint8_t length, uint32_t timeoutMs, bool*callbackFlag){
    // Time keeping variables
    uint32_t elapsedTimeMs = 0;
    const uint32_t msIncr = 10;

    // Reset callback flag
    *callbackFlag=false;

    // Send frame
    if(tx(frame, length)!=length)
        //return false;

    // ISM tick until timeout is reached or ping back received
    while (elapsedTimeMs<timeoutMs && !*callbackFlag){
        ism_tick();
        delayMs(msIncr);
        elapsedTimeMs+=msIncr;
    }

    return *callbackFlag;
}


bool Node::net_ping(uint32_t timeoutMs){
    // Send ping command
    // Classic network frame
    uint8_t frame[2] = {NETWORK_PROTOCOL_ID, NETWORK_PING};

    printNetFrame(frame, 2, true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &pingCallback);

    return pingCallback;
}

bool Node::net_getUid(uint32_t timeoutMs){
    // Classic network frame
    uint8_t frame[2] = {NETWORK_PROTOCOL_ID, NETWORK_GETUID};

    printNetFrame(frame, 2, true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &uidCallback);

    return uidCallback;
}

bool Node::net_setAddr(uint8_t newAddr){
    // Classic network frame
    uint8_t frame[3] = {NETWORK_PROTOCOL_ID, NETWORK_SETADDR, newAddr};
    printNetFrame(frame, 3, true);
    // Send frame without timeout as this command is not confirmed
    tx(frame, 3);
    oldAddress=address;
    address=newAddr;

    return true;
}


bool Node::net_setAddrAgain(uint8_t maxTries, uint32_t timeoutMs){
    // Revert to old address state
    uint8_t newAddress=address;
    address=oldAddress;

    // Ping maxTries times
    for(uint8_t pings=0;pings<maxTries;pings++){
        // if node doesn't respond to ping with new address
        if( !net_ping(timeoutMs) ){
            // Try changing address again
            net_setAddr(newAddress);
        }else{
            // else nothing more to worry about
            return true;
        }
    }
    return false;
}

bool Node::net_setGroup(uint32_t newGroup){
    // Classic network frame
    uint8_t frame[6] = {NETWORK_PROTOCOL_ID, NETWORK_SETGROUP};
    memcpy(&frame[2], &newGroup, sizeof(newGroup));
    printNetFrame(frame, 6, true);
    // Send frame without timeout as this command is not confirmed
    tx(frame, 6);
    group=newGroup;

    return true;
}

bool Node::net_disconnect(uint32_t timeoutMs){
    // Classic network frame
    uint8_t frame[2] = {NETWORK_PROTOCOL_ID, NETWORK_DISCONNECT};

    printNetFrame(frame, 2, true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &disconnectStatus);

    return disconnectStatus;
}

bool Node::net_getProtocols(uint32_t timeoutMs){
    // Classic network frame
    uint8_t frame[2] = {NETWORK_PROTOCOL_ID, NETWORK_GET_PROTOCOLS};

    printNetFrame(frame, 2, true);
    // Send frame with timeout
    txTimeout(frame, 2, timeoutMs, &protocolsCallback);

    return protocolsCallback;
}

bool Node::pingStatus(){
    return pingCallback;
}

bool Node::uidStatus(){
    return uidCallback;
}

bool Node::protocolsStatus(){
    return protocolsCallback;
}

bool Node::isConnected(){
    return !disconnectStatus;
}

bool Node::isLeaseValid(){
    uint32_t leaseDurationS=NETWORK_LEASE_UNIT_MINUTES*60*leaseDuration;
    uint32_t elapsedTimeS = ((uint32_t)time(NULL)) - leaseStartTime;
#ifdef DEBUG_DORA
    cout<<"Lease status for node "; show(); cout<<" : "<<to_string(elapsedTimeS)<<"/"<<to_string(leaseDurationS)<<"s"<<endl;
#endif
    return elapsedTimeS<leaseDurationS*1.3;
}


void Node::rxCallback(const uint8_t* data, uint8_t size){
    printFrame(data, size, false);
    // PROTOCOL DISPATCH
    // Get protocol ID from first frame
    if(size>=2){
        switch(data[0]){
            case NETWORK_PROTOCOL_ID:
                netCmdCallback(data+1, size-1);
                break;
            case APP_PROTOCOL_ID:
                appCmdCallback(data+1, size-1);
                break;
            case APP_ERR_PROTOCOL_ID:
                appErrCallback(data+1, size-1);
                break;
            case DATA_PROTOCOL_ID:
                dataCallback(data+1, size-1);
                break;
            default:
                // Unrecognized frame
                break;
        }
    }else{
        // Frame too short
    }
}


void Node::netCmdCallback(const uint8_t* data, uint8_t size){
    uint8_t cmd = data[0];
    uint8_t dataSize = size-1;
    uint8_t dataIndex = 1;
    uint8_t answer[3]={NETWORK_PROTOCOL_ID};
    uint8_t answerLength=2;

    printNetFrame(data, size, false);
    // Command dispatch
    switch(cmd){
        case NETWORK_PING:
            // Set ping result
            pingCallback=true;
            break;
        case NETWORK_GETUID:
            if (dataSize >= NETWORK_UID8_WIDTH){
                memcpy(&uid, &data[dataIndex], NETWORK_UID8_WIDTH);
                dataSize-=NETWORK_UID8_WIDTH;
                dataIndex+=NETWORK_UID8_WIDTH;
                uidCallback=true;
            }else{
                // Frame too short
            }
            break;
        case NETWORK_SETADDR:
            // Do nothing (unconfirmed)
            break;
        case NETWORK_SETGROUP:
            // Do nothing (unconfirmed)
            break;
        case NETWORK_DISCONNECT:
            // Confirm disconnect
            disconnectStatus=true;
            answer[1]=NETWORK_DISCONNECT;
            tx(answer,answerLength);
            break;
        case NETWORK_RENEW_LEASE:
            leaseStartTime=(uint32_t) time(NULL);
            // Confirm lease renewal
            answer[1]=NETWORK_RENEW_LEASE;
            answer[2]=leaseDuration;
            answerLength=3;
            tx(answer,answerLength);
#ifdef DEBUG_DORA
            cout<<"Lease renewed for node ";show();cout<<endl;
#endif
            break;
        case NETWORK_GET_PROTOCOLS:
            if(dataSize >= NETWORK_PROTOCOL_ID){
                protocols=data[dataIndex];
                protocolsCallback=true;
            }
            break;
        default:
            // Error handling
            break;
    }
}


void Node::printFrame(const uint8_t * buffer, uint8_t size, bool dir){
#ifdef DEBUG_RXTX
    if(dir)
        cout<<"Node (address="<<to_string(address)<<") TX: ";
    else
        cout<<"Node (address="<<to_string(address)<<") RX: ";

    printBufferHex(buffer, size);
#endif
}

void Node::printNetFrame(const uint8_t * buffer, uint8_t size, bool dir){
#ifdef DEBUG_NET
    if(dir)
        cout<<"Node NET TX: ";
    else
        cout<<"Node NET RX: ";

    printBufferHex(buffer, size);
#endif
}

void Node::printAppFrame(const uint8_t * buffer, uint8_t size, bool dir){
#ifdef DEBUG_APP
    cout<<"WARNING: Node is not configured for APP commands"<<endl;
    if(dir)
        cout<<"Node APP TX: ";
    else
        cout<<"Node APP RX: ";

    printBufferHex(buffer, size);
#endif
}

void Node::printDataFrame(const uint8_t * buffer, uint8_t size, bool dir){
#ifdef DEBUG_DATA
    cout<<"WARNING: Node is not configured for DATA commands"<<endl;
    if(dir)
        cout<<"Node DATA TX: ";
    else
        cout<<"Node DATA RX: ";

    printBufferHex(buffer, size);
#endif
}
