#include "wpanManager.h"

// Call constructor with power initialization with default values from ism server
wpanManager::wpanManager() :
wpanManager{ism_server_get_power(),ism_server_get_power_dbm()} {}

// Main constructor
wpanManager::wpanManager(uint8_t power_, uint8_t power_dbm_)
{
    txpower=power_;
    txpower_dbm=power_dbm_;
    ism_server_init(txpower,txpower_dbm);
    ism_get_uid(uid, ISM_UID_SIZE);
}

// Call empty constructor and initialize node list after
wpanManager::wpanManager(vector<Node> nodeList_) : wpanManager() {
    staticBaseNodes=nodeList_;
    for(uint8_t i=0; i<staticBaseNodes.size(); i++){
      pStaticNodes.push_back(&staticBaseNodes.at(i));
    }
    pNodes=pStaticNodes;
#ifdef DEBUG_WPANMANAGER
    cout<<"WPAN manager node vector size: "<<pNodes.size()<<endl;
    cout<<"Content: "<<endl;
    printNodes();
#endif
}

// Call main constructor and initialize node list after
wpanManager::wpanManager(vector<Node> nodeList_,
                         uint8_t power_,
                         uint8_t power_dbm_) : wpanManager{power_, power_dbm_}
{
    staticBaseNodes=nodeList_;
    for(uint8_t i=0; i<staticBaseNodes.size(); i++){
      pStaticNodes.push_back(&staticBaseNodes.at(i));
    }
    pNodes=pStaticNodes;
#ifdef DEBUG_WPANMANAGER
    cout<<"WPAN manager node vector size: "<<pNodes.size()<<endl;
    cout<<"Content: "<<endl;
    printNodes();
#endif
}


vector<Node*> wpanManager::getNodeList()
{
    return pNodes;
}

vector<Node*> wpanManager::getStaticNodeList()
{
    return pStaticNodes;
}

vector<PowerNode*> wpanManager::getPowerNodeList()
{
    vector<PowerNode*> retVec;
    for(int i=0;i<staticPowerNodes.size();i++){
        retVec.push_back(&staticPowerNodes.at(i));
    }
    for(int i=0;i<dynPowerNodes.size();i++){
        retVec.push_back(&dynPowerNodes.at(i));
    }
    return retVec;
}

vector<DataNode*> wpanManager::getDataNodeList()
{
    vector<DataNode*> retVec;
    for(int i=0;i<staticDataNodes.size();i++){
        retVec.push_back(&staticDataNodes.at(i));
    }
    for(int i=0;i<dynDataNodes.size();i++){
        retVec.push_back(&dynDataNodes.at(i));
    }
    return retVec;
}

void wpanManager::clearNodeLists(){
    clearDynamicNodeList();
    clearStaticNodeList();
    pNodes.clear();
}
void wpanManager::clearDynamicNodeList(){
    ism_server_wakeup_group(0xFFFFFFFF);
    tick(20);
    for(uint8_t i=0; i<pDynNodes.size(); i++){
#ifdef DEBUG_WPANMANAGER
        cout<<"Disconnecting node with address ";
        cout<<to_string(pDynNodes.at(i)->getAddr());
        cout<<", group ";
        cout<<to_string(pDynNodes.at(i)->getGroup())<<endl;
#endif
        pDynNodes.at(i)->wakeup();
        tick(20);
        pDynNodes.at(i)->net_disconnect(0);
        tick(20);

    }
    pDynNodes.clear();
    dynBaseNodes.clear();
    dynPowerNodes.clear();
    dynDataNodes.clear();
    for(uint8_t i=0; i<25; i++){
        tick(20);
    }
}
void wpanManager::clearStaticNodeList(){
    for(uint8_t i=0; i<pStaticNodes.size(); i++){
        pDynNodes.at(i)->wakeup();
        tick(20);
        pStaticNodes.at(i)->net_disconnect(0);
        tick(20);
    }
    pStaticNodes.clear();
    staticBaseNodes.clear();
    staticPowerNodes.clear();
    staticDataNodes.clear();
}

void wpanManager::tick(uint32_t delayMs_){
#ifdef DEBUG_TICKS
    cout<<"WPAN MANAGER TICK"<<endl;
#endif
    ism_tick();
    delayMs(delayMs_);
}

void wpanManager::rxHandler(const uint8_t* data, uint8_t size, uint8_t source){
    bool found=false;
    uint8_t nodeAddr=0;

#ifdef DEBUG_WPANMANAGER
    cout<<"RX HANDLER"<<endl;
    cout<<"Source: "<<to_string(source)<<endl;
    cout<<"Data: ";
    printBufferHex(data, size);
#endif
    // Iterate through list to find node that sent the packet
    for(uint8_t i=0; i<pNodes.size(); i++){
        nodeAddr=pNodes[i]->getAddr();

#ifdef DEBUG_WPANMANAGER
        cout<<"Checking node "<<to_string(i)<<", address "<<to_string(nodeAddr)<<endl;
#endif

        if(nodeAddr==source){
            pNodes[i]->rxCallback(data,size);
            found=true;

#ifdef DEBUG_WPANMANAGER
            cout<<"Found destination node"<<endl;
#endif
        }
    }
    /*
    // If none found, it may come from a node's old address
    // In this case, try to set the node's address again
    if(!found){
        rxHandler_oldAddr(data, size, source);
    }*/
    // TODO handle case where node is in node list but sends DORA request
    //if(!found){
        if(size>=2){
            if(data[0]==NETWORK_PROTOCOL_ID &&
                (data[1]==NETWORK_DISCOVER||data[1]==NETWORK_REQUEST) ){
#ifdef DEBUG_DORA
                cout<<"DORA function called"<<endl;
#endif
                dora(data+1,size-1);
            }
        }
    //}
}


void wpanManager::rxHandler_oldAddr(const uint8_t* data, uint8_t size, uint8_t source){
    for(uint8_t i=0; i<pNodes.size(); i++){
        // If source matches an old address
        if(pNodes[i]->getOldAddr()==source){
            // execute RX callback as usual
            pNodes[i]->rxCallback(data,size);
            if(!(pNodes[i]->net_setAddrAgain(maxPingRetries, netCmdTimeout))){
                // If all pings were unsuccessful
                // find and remove node from connectedNodes list
// TODO fix the whole connected/unconnected/nodes nonsensical mess
                vector<Node*>::iterator it;/*
                it = find(connectedNodes.begin(),connectedNodes.end(),nodes[i]);
                if(it!=connectedNodes.end()){
                    connectedNodes.erase(it);
                }*/
            }
        }
    }
}

void wpanManager::rebuildNodeLists(){
    rebuildStaticNodeLists();
    rebuildDynNodeLists();
}

void wpanManager::rebuildStaticNodeLists(){
    // Remove existing pointers from pNodes
    for(int i=0; i<pStaticNodes.size(); i++){
        for(int j=pNodes.size()-1; j>=0; j--){
            if(pNodes.at(j)==pStaticNodes.at(i)){
                pNodes.erase(pNodes.begin()+j);
            }
        }
    }
    // Clear pStaticNodes array
    pStaticNodes.clear();

    // Rebuild pStaticNodes array
    for(int i=0;i<staticBaseNodes.size();i++){
        pStaticNodes.push_back(&(staticBaseNodes.at(i)));
    }
    for(int i=0;i<staticPowerNodes.size();i++){
        pStaticNodes.push_back(&(staticPowerNodes.at(i)));
    }
    for(int i=0;i<staticDataNodes.size();i++){
        pStaticNodes.push_back(&(staticDataNodes.at(i)));
    }

    // Refill pNodes array
    for(int i=0;i<pStaticNodes.size();i++){
        pNodes.push_back(pStaticNodes.at(i));
    }
#ifdef DEBUG_WPANMANAGER
    printNodes();
#endif
}

void wpanManager::rebuildDynNodeLists(){
    // Remove existing pointers from pNodes
    for(int i=0; i<pDynNodes.size(); i++){
        for(int j=pNodes.size()-1; j>=0; j--){
            if(pNodes.at(j)==pDynNodes.at(i)){
                pNodes.erase(pNodes.begin()+j);
            }
        }
    }
    // Clear pDynNodes array
    pDynNodes.clear();

    // Rebuild pDynNodes array
    for(int i=0;i<dynBaseNodes.size();i++){
        pDynNodes.push_back(&(dynBaseNodes.at(i)));
    }
    for(int i=0;i<dynPowerNodes.size();i++){
        pDynNodes.push_back(&(dynPowerNodes.at(i)));
    }
    for(int i=0;i<dynDataNodes.size();i++){
        pDynNodes.push_back(&(dynDataNodes.at(i)));
    }

    for(int i=0;i<pDynNodes.size();i++){
        pNodes.push_back(pDynNodes.at(i));
    }
#ifdef DEBUG_WPANMANAGER
    printNodes();
#endif
}




void wpanManager::updateDynamicNodeTypes(){
    uint32_t awakeGroups=ism_server_get_awake();
    uint8_t nodeProtocols=0;
    uint8_t nodeTypeProtocols=0;
    // Wake up all groups but DORA group
    ism_server_wakeup_group(~NETWORK_DORA_GROUP);
    // Send net_getprotocols to all nodes
#ifdef DEBUG_WPANMANAGER
        cout<<"Send get protocols to all unknown dynamic nodes"<<endl;
#endif
    for(int i=0; i<dynBaseNodes.size(); i++){
        if(dynBaseNodes.at(i).getProtocols()==0){
#ifdef DEBUG_WPANMANAGER
            cout<<"Dynamic node ";
            dynBaseNodes.at(i).show();
            cout<<endl;
#endif
            dynBaseNodes.at(i).net_getProtocols(0);
            tick(20);
        }
    }
    // Allow time to answer
    for(uint8_t i=0; i<50; i++){
        tick(20);
    }

    // Block nodes from sending data during list update
    ism_server_unwake_groups();


    // Reassign nodes so that class matches protocols
    for(int i=dynBaseNodes.size()-1; i>=0; i--){
        nodeProtocols=dynBaseNodes.at(i).getProtocols();
        nodeTypeProtocols=dynBaseNodes.at(i).getNodeTypeProtocols();
#ifdef DEBUG_WPANMANAGER
        cout<<"Find type for ";
        dynBaseNodes.at(i).show();
        cout<<endl;
        cout<<"Protocols for node type: "<<to_string(nodeTypeProtocols)<<endl;
        cout<<"Node has announced protocols: "<<to_string(nodeProtocols)<<endl;
#endif
        if(nodeProtocols!=nodeTypeProtocols){
            switch(nodeProtocols){
                case NETWORK_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // DataNode
#ifdef DEBUG_WPANMANAGER
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    dynDataNodes.push_back(DataNode{dynBaseNodes.at(i)});
                    dynBaseNodes.erase(dynBaseNodes.begin()+i);
                    break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID:
                    // PowerNode
#ifdef DEBUG_WPANMANAGER
                    cout<<"Move node "<<to_string(i)<<" to PowerNodes"<<endl;
#endif
                    dynPowerNodes.push_back(PowerNode{dynBaseNodes.at(i)});
                    dynBaseNodes.erase(dynBaseNodes.begin()+i);
                break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // Data Node
#ifdef DEBUG_WPANMANAGER
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    dynDataNodes.push_back(DataNode{dynBaseNodes.at(i)});
                    dynBaseNodes.erase(dynBaseNodes.begin()+i);
                break;
                default:
                    // Keep node in dynBaseNodes
                break;
            }
        }
    }
    rebuildDynNodeLists();

    // Put groups back to previous state
    ism_server_wakeup_group(awakeGroups);
}

void wpanManager::updateStaticNodeTypes(){
    uint32_t awakeGroups=ism_server_get_awake();
    uint8_t nodeProtocols=0;
    uint8_t nodeTypeProtocols=0;
    // Wake up all groups but DORA group
    ism_server_wakeup_group(~NETWORK_DORA_GROUP);
    // Send net_getprotocols to all nodes
#ifdef DEBUG_WPANMANAGER
        cout<<"Send get protocols to all unknown static nodes"<<endl;
#endif
    for(int i=0; i<staticBaseNodes.size(); i++){
        if(staticBaseNodes.at(i).getProtocols()==0){
#ifdef DEBUG_WPANMANAGER
            cout<<"Send command to ";
            staticBaseNodes.at(i).show();
            cout<<endl;
#endif
            staticBaseNodes.at(i).net_getProtocols(0);
            tick(20);
        }
    }
    // Allow time to answer
    for(uint8_t i=0; i<50; i++){
        tick(20);
    }

    // Block nodes from sending data during list update
    ism_server_unwake_groups();


    // Reassign nodes so that class matches protocols
    for(int i=staticBaseNodes.size()-1; i>=0; i--){
        nodeProtocols=staticBaseNodes.at(i).getProtocols();
        nodeTypeProtocols=staticBaseNodes.at(i).getNodeTypeProtocols();
#ifdef DEBUG_WPANMANAGER
        cout<<"Find type for ";
        staticBaseNodes.at(i).show();
        cout<<endl;
        cout<<"Protocols for node type: "<<to_string(nodeTypeProtocols)<<endl;
        cout<<"Node has announced protocols: "<<to_string(nodeProtocols)<<endl;
#endif
        if(nodeProtocols!=nodeTypeProtocols){
            switch(nodeProtocols){
                case NETWORK_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // DataNode
#ifdef DEBUG_WPANMANAGER
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    staticDataNodes.push_back(DataNode{staticBaseNodes.at(i)});
                    staticBaseNodes.erase(staticBaseNodes.begin()+i);
                    break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID:
                    // PowerNode
#ifdef DEBUG_WPANMANAGER
                    cout<<"Move node "<<to_string(i)<<" to PowerNodes"<<endl;
#endif
                    staticPowerNodes.push_back(PowerNode{staticBaseNodes.at(i)});
                    staticBaseNodes.erase(staticBaseNodes.begin()+i);
                break;
                case NETWORK_PROTOCOL_ID|APP_PROTOCOL_ID|APP_ERR_PROTOCOL_ID|DATA_PROTOCOL_ID:
                    // Data Node
#ifdef DEBUG_WPANMANAGER
                    cout<<"Move node "<<to_string(i)<<" to DataNodes"<<endl;
#endif
                    staticDataNodes.push_back(DataNode{staticBaseNodes.at(i)});
                    staticBaseNodes.erase(staticBaseNodes.begin()+i);
                break;
                default:
                    // Keep node in staticBaseNodes
                break;
            }
        }
    }
    rebuildStaticNodeLists();
    // Put groups back to previous state
    ism_server_wakeup_group(awakeGroups);
}


void wpanManager::updateNodeTypes(){
    updateStaticNodeTypes();
    updateDynamicNodeTypes();
}

void wpanManager::printNodes(){
    if(pNodes.empty()) return;
    for(int i=0; i<pNodes.size(); i++){
        cout<<to_string(i+1)<<" - ";
        pNodes.at(i)->show();
        cout<<endl;
    }
}

void wpanManager::printStaticNodes(){
    for(int i=0; i<pStaticNodes.size(); i++){
        cout<<to_string(i+1)<<" - ";
        pStaticNodes[i]->show();
        cout<<endl;
    }
}

void wpanManager::startDynamicDiscovery(){
    ism_server_wakeup_group(NETWORK_NACK_GROUP);
}

void wpanManager::stopDynamicDiscovery(){
    ism_server_unwake_group(NETWORK_NACK_GROUP);
}

// PRIVATE METHODS
uint8_t wpanManager::dora(const uint8_t* data, uint8_t size){
    /*
     * DHCP-like registration:
        Unregistered clients are in group 0x80000000 by default
        Clients have a random default address between 0xF0 and 0xFE
        Server wakes unregistered group
        Clients broadcast a message asking for addresses, giving their uids
        Server broadcasts a message assigning an address to a uid
        Client gets message, compares with its uid and sets address if correct
    */
    /* Frame data:
     * NETWORK_PROTOCOL_ID
     * NETWORK_CMD
     * source UID
     * destination UID
     * NETWORK_CMD_DATA
     *
     * DATA per command
     * DISCOVER:    none
     * OFFER:       address
     * REQUEST:     address     group
     * ACK:         address     group   lease duration (0 if NACK)
     */
    uint8_t dataIndex=0;
    uint8_t dataSize=size;
    uint8_t sourceUID[NETWORK_UID8_WIDTH];
    uint8_t destinationUID[NETWORK_UID8_WIDTH];
    uint8_t cmd;
    uint8_t frame[NETWORK_DORA_MAX_FRAME_SIZE]={0};
    uint8_t frameLength=0;
    uint8_t addr=0; // node temp address
    uint8_t newAddr=1; // Default for OFFER command
    uint32_t group=NETWORK_NACK_GROUP;
    uint8_t cmdData[sizeof(newAddr)+sizeof(group)+sizeof(leaseDuration)]={0};
    uint8_t cmdDataLength=0;
    bool addrUsed=false;

#ifdef DEBUG_DORA
            cout<<"DORA handler: "<<endl;
            cout<<"Data: ";
            printBufferHex(data, size);
#endif
    if(dataSize>=1){
        cmd=data[dataIndex];
        dataIndex++;
        dataSize--;
    }else{
        cmd=0;
    }
    // Decode source UID regardless of destination UID
    if(dataSize>=NETWORK_UID8_WIDTH){
        memcpy(sourceUID, &data[dataIndex], NETWORK_UID8_WIDTH);
        dataIndex+=NETWORK_UID8_WIDTH;
        dataSize-=NETWORK_UID8_WIDTH;
    }
    if(dataSize>=NETWORK_UID8_WIDTH){
        memcpy(destinationUID, &data[dataIndex], NETWORK_UID8_WIDTH);
        dataIndex+=NETWORK_UID8_WIDTH;
        dataSize-=NETWORK_UID8_WIDTH;
    }


    switch(cmd){
        case NETWORK_DISCOVER:

#ifdef DEBUG_DORA
            cout<<"RX DORA DISCOVER "<<endl;
#endif
            // If DISCOVER command
            // Find first free address in nodes vector
            for(newAddr=1;newAddr<255;newAddr++){
                // Compare newAddr with all existing addresses
                for(uint8_t i=0;i<pNodes.size();i++){
                    addr=pNodes.at(i)->getAddr();
                    if(newAddr==pNodes.at(i)->getAddr()){
                        // If address is used, try next address
                        addrUsed=true;
                        break;
                    }
                }
                if(!addrUsed){
                    // If newAddr is not used in the node list, use it
                    break;
                }
            }

            // OFFER free address
            cmdData[0]=newAddr;
            cmdDataLength++;
            frameLength=buildDoraFrame(frame,ISM_MAX_DATA_SIZE, NETWORK_OFFER, sourceUID, cmdData, cmdDataLength);
            storeOffer(newAddr, sourceUID);
            // Broadcast offer
#ifdef DEBUG_DORA
            cout<<"TX DORA OFFER"<<endl;
            cout<<"Data: ";
            printBufferHex(frame, frameLength);
#endif
            ism_broadcast(NETWORK_DORA_GROUP, 1, frame, frameLength);
            break;

        case NETWORK_REQUEST:
#ifdef DEBUG_DORA
            cout<<"RX DORA REQUEST "<<endl;
#endif
            // If REQUEST command
            // Get address, group
            if(dataSize>=(sizeof(newAddr)+sizeof(group))){
                addr=data[dataIndex];
                dataIndex++;
                memcpy(&group, &data[dataIndex], NETWORK_GROUP_WIDTH);
                dataIndex+=NETWORK_GROUP_WIDTH;
            }
            // Check destination UID matches own UID
            if(checkOfferMatch(addr,sourceUID)){
                // Build frame with found address, requested group and lease
                cmdData[0]=addr;
                memcpy(&cmdData[NETWORK_ADDR_WIDTH],&group,NETWORK_GROUP_WIDTH);
                cmdData[NETWORK_ADDR_WIDTH+NETWORK_GROUP_WIDTH]=leaseDuration;
            }else{
                // Build frame with NACK address, NACK group and 0 lease
                cmdData[0]=NETWORK_NACK_BASE_ADDR+addressOffers.size();
                memcpy(&cmdData[NETWORK_ADDR_WIDTH],&group,NETWORK_GROUP_WIDTH);
                cmdData[NETWORK_ADDR_WIDTH+NETWORK_GROUP_WIDTH]=0; // No lease
            }
            cmdDataLength+=NETWORK_ADDR_WIDTH+NETWORK_GROUP_WIDTH+NETWORK_LEASE_WIDTH;
            // Send ACK message with lease duration
            frameLength=buildDoraFrame(frame,ISM_MAX_DATA_SIZE, NETWORK_ACK, sourceUID, cmdData, cmdDataLength);
#ifdef DEBUG_DORA
            cout<<"TX DORA ACK"<<endl;
            cout<<"Data: ";
            printBufferHex(frame, frameLength);
#endif
            ism_broadcast(NETWORK_DORA_GROUP, 1, frame, frameLength);

            // Create node with new address and group

            // Register node in dynBaseNodes array
            dynBaseNodes.push_back(Node{addr, group, leaseDuration});
            // Add reference to
            pNodes.push_back(&(dynBaseNodes.back()));
            pDynNodes.push_back(&(dynBaseNodes.back()));

            break;
        default:
            // NETWORK_DORA_ERR message, no data
#ifdef DEBUG_DORA
            cout<<"DORA ERROR "<<endl;
#endif
            cmdDataLength=0;
            frameLength=buildDoraFrame(frame,ISM_MAX_DATA_SIZE, NETWORK_DORA_ERR, sourceUID, cmdData, cmdDataLength);
            ism_broadcast(NETWORK_DORA_GROUP, 1, frame, frameLength);
            break;
    }


#ifdef DEBUG_DORA
            cout<<"EXIT DORA handler"<<endl;
#endif

    return 0;
}


void wpanManager::storeOffer(uint8_t address, uint8_t * uid){
    addressOffers.push_back(address);
    offerUidHashes.push_back(hashUid(uid));
}


bool wpanManager::checkOfferMatch(uint8_t address, uint8_t * uid){
    uint8_t hash=hashUid(uid);

    for(uint8_t i=0;i<offerUidHashes.size();i++){
        // If match is found
        if(hash==offerUidHashes.at(i) && address==addressOffers.at(i)){
            // Delete stored pair and return true
            offerUidHashes.erase(offerUidHashes.begin()+i);
            addressOffers.erase(addressOffers.begin()+i);
            return true;
        }
    }

    return false;
}


uint8_t wpanManager::hashUid(uint8_t* uid){
    uint8_t hash=0;
    for(uint8_t i=0;i<NETWORK_UID8_WIDTH;i++){
        hash^=uid[i];
    }
    return hash;
}


uint8_t wpanManager::buildDoraFrame(uint8_t * buffer, uint8_t maxSize, uint8_t cmd, uint8_t * destUID, uint8_t * data, uint8_t dataSize){
    /* Frame data:
     * NETWORK_PROTOCOL_ID
     * NETWORK_CMD
     * source UID
     * destination UID
     * NETWORK_CMD_DATA
     */
    uint8_t frameLength=0;

    // Check buffer is big enough
    if(maxSize>=NETWORK_MAX_FRAME_SIZE){
        // Network protocol ID
        buffer[0]=NETWORK_PROTOCOL_ID;
        // Network cmd
        buffer[1]=cmd;
        frameLength+=2;
#ifdef DEBUG_DORA
            cout<<"BUILD DORA FRAME"<<endl;
            cout<<"SRC UID: ";
            printBufferHex(uid, NETWORK_UID8_WIDTH);
            cout<<"DEST UID: ";
            printBufferHex(destUID, NETWORK_UID8_WIDTH);
            cout<<"CMD DATA: ";
            printBufferHex(data, dataSize);
#endif
        // Source UID
        memcpy(&buffer[frameLength],uid,NETWORK_UID8_WIDTH);
        frameLength+=NETWORK_UID8_WIDTH;
        // Dest UID
        memcpy(&buffer[frameLength],destUID,NETWORK_UID8_WIDTH);
        frameLength+=NETWORK_UID8_WIDTH;
        // Cmd data
        memcpy(&buffer[frameLength],data,dataSize);
        frameLength+=dataSize;
    }

    return frameLength;
}




/*
wpanManager::~wpanManager()
{

}*/
/*
uint8_t wpanManager::staticDiscovery(vector<Node> nodelist){

    return connectedNodes.size();
}*/
