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
}

/*
Node::Node(const Node& other)
{

}*/

Node::~Node()
{

}


ostream& operator<<(ostream & out, const Node & node){
    out<<"Address: "<<to_string(node.address);
    out<<" Group: "<<to_string(node.group);
    return out;
}
/*
Node& Node::operator=(const Node& other)
{

}

bool Node::operator==(const Node& other) const
{

}

bool Node::operator!=(const Node& other) const
{

}*/


// GETTERS

uint8_t Node::getUid(uint8_t * buffer, uint8_t size){
    if(size>=NODE_UID8_WIDTH){
        memcpy(buffer, uid, NODE_UID8_WIDTH);
        return NODE_UID8_WIDTH;
    }
    return 0;
}
uint8_t Node::getAddr(){
    return address;
}
uint8_t Node::getOldAddr(){
    return oldAddress;
}
uint8_t Node::getGroup(){
    return group;
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

/*
bool Node::initConnection(){
    // Get node uid

    return true;
}*/


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


bool Node::pingStatus(){
    return pingCallback;
}

bool Node::uidStatus(){
    return uidCallback;
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
            default:
                // TODO Error handling
                break;
        }
    }else{
        // Frame too short
        // TODO add error handling
    }
}


void Node::netCmdCallback(const uint8_t* data, uint8_t size){
    uint8_t cmd = data[0];
    uint8_t dataSize = size-1;
    uint8_t dataIndex = 1;

    printNetFrame(data, size, false);
    // CMD DISPATCH
    // nice TODO: develop generic version of this switch case using an array
    // to define network commands (network.c/h)

    if(cmd&NETWORK_PING){
        // Set ping result
        pingCallback=true;
    }else if(cmd&NETWORK_GETUID){
        if (dataSize >= NODE_UID8_WIDTH){
            memcpy(&uid, &data[dataIndex], NODE_UID8_WIDTH);
            dataSize-=NODE_UID8_WIDTH;
            dataIndex+=NODE_UID8_WIDTH;
            uidCallback=true;
        }else{
            // TODO Error handling
        }
    }else if(cmd&NETWORK_SETADDR){
        if (dataSize>=1){
            // Do nothing (unconfirmed)
        }else{
            // TODO Error handling
        }
    }
}


void Node::printFrame(const uint8_t * buffer, uint8_t size, bool dir){
#ifdef DEBUG_RXTX
    if(dir)
        cout<<"Node TX: ";
    else
        cout<<"Node RX: ";

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
